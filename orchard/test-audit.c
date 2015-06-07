#include "ch.h"
#include "chprintf.h"
#include "orchard.h"

#include "storage.h"
#include "test-audit.h"

#include <string.h>

static void init_audit(uint32_t block) {
  struct auditLog log;

  log.signature = AUDIT_SIGNATURE;
  log.version = AUDIT_VERSION;
  log.entry_count = 0;
  log.firstEntry.runs = 0;
  log.firstEntry.type = 0;
  log.firstEntry.result = 0;
  memset(log.firstEntry.testName, 0, sizeof(log.firstEntry.testName));
    
  storagePatchData(block, (uint32_t *) &log, AUDIT_OFFSET, sizeof(struct auditLog));
}

void auditStart(void) {
  const struct auditLog *log;

  log = (const struct auditLog *) storageGetData(AUDIT_BLOCK);

  if( log->signature != AUDIT_SIGNATURE ) {
    init_audit(AUDIT_BLOCK);
  }
  if( sizeof(auditEntry) != (TEST_NAME_LENGTH + 2 + 1 + 1) ) {
    chprintf(stream, "WARNING: Test audit entry layout is broken due to compiler padding.\n\r" );
  }
  if( log->version != AUDIT_VERSION ) {
    chprintf(stream, "WARNING: Version mismatch on audit log. Behavior will be unpredictable.\n\r" );
  }
  
}

// check audit log to see if all tests of test_type have passed
int32_t auditCheck(uint32_t test_type) {
  return -1;
}

const auditEntry *find_audit_entry(const char *name, OrchardTestType type) {
  const struct auditLog *log;
  const auditEntry *entry = NULL;
  uint32_t i;

  log = (const struct auditLog *) storageGetData(AUDIT_BLOCK);

  if( log->entry_count == 0 )
    return NULL;
  
  entry = &(log->firstEntry);
  for( i = 0; i < log->entry_count; i++ ) {
    if( ((OrchardTestType) entry->type == type) &&
	(strncmp(entry->testName, name, TEST_NAME_LENGTH) == 0) ) {
      return entry;
    }
    entry++;
  }
  return NULL;
}

void auditPrintLog(void) {
  const struct auditLog *log;
  uint32_t i;
  const auditEntry *entry;
  
  log = (const struct auditLog *) storageGetData(AUDIT_BLOCK);
  chprintf(stream, "entry_count: %d\n\r", log->entry_count);
  chprintf(stream, "signature: %08x\n\r", log->signature);
  chprintf(stream, "version: %d\n\r", log->version);

  chprintf(stream, "Log (runs type result | name)\n\r", log->version);
  entry = &(log->firstEntry);
  for( i = 0; i < log->entry_count; i++ ) {
    chprintf(stream, "  %5d %3d %3d | %s\n\r", entry->runs, entry->type, entry->result, entry->testName);
    entry++;
  }
}

void auditUpdate(const char *name, OrchardTestType type, OrchardTestResult result ) {
  // finds an entry and updates it;
  // if the entry doesn't exist, it creates it.
  uint32_t auditLogSize = sizeof(auditLog);  // size of an empty audit log
  const auditEntry *entry;
  auditEntry newEntry;
  const struct auditLog *log;
  uint32_t newCount;

  log = (const struct auditLog *) storageGetData(AUDIT_BLOCK);
  
  entry = find_audit_entry(name, type);
  
  if( entry == NULL ) {
    auditLogSize +=  sizeof(auditEntry) * (log->entry_count > 0 ? log->entry_count - 1 : 0);

    if( (auditLogSize + sizeof(auditEntry)) >= BLOCK_SIZE ) {
      chprintf( stream, "AUDIT LOG ERROR: out of space, can't add new log entry.\n\r" );
      return;
    }
    // make a new one
    newEntry.runs = 1;
    newEntry.type = type;
    newEntry.result = result;
    strncpy(newEntry.testName, name, TEST_NAME_LENGTH);

    // tack it onto the end
    storagePatchData(AUDIT_BLOCK, (uint32_t *) &newEntry,
		     (sizeof(auditLog) - sizeof(auditEntry)) + log->entry_count * sizeof(auditEntry),
		     sizeof(auditEntry));
    // update the entry_count
    newCount = log->entry_count + 1;
    storagePatchData(AUDIT_BLOCK, &newCount, 0, sizeof(uint32_t)); 
  } else {
    // update the existing entry
    newEntry.runs = entry->runs + 1;
    newEntry.type = entry->type;
    newEntry.result = result;
    strncpy(newEntry.testName, entry->testName, TEST_NAME_LENGTH);
    
    storagePatchData(AUDIT_BLOCK, (uint32_t *) &newEntry,
		     (uint32_t) entry - (uint32_t) log,
		     sizeof(auditEntry));
  }
}
