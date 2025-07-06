# TransactionBuilder

The `TransactionBuilder` class provides a fluent interface for constructing well-formed transactions in the KeePassXC transactional system. It helps prevent common errors like mixing transaction types, missing required parameters, and invalid transaction combinations.

## Features

- **Fluent Interface**: Chain method calls for readable transaction construction
- **Type Safety**: Compile-time validation of transaction structure
- **Error Prevention**: Built-in validation prevents common mistakes
- **Comprehensive Validation**: Detailed error messages for debugging

## Quick Start

### Basic Entry Update

```cpp
#include "TransactionBuilder.h"

auto entryId = QUuid::createUuid();

auto transaction = TransactionBuilder::forEntry(entryId)
                      .withDescription("Update login credentials")
                      .updateEntry()
                      .withTitle("GitHub Account")
                      .withUsername("john.doe@company.com")
                      .withPassword("newSecurePassword123!")
                      .withUrl("https://github.com")
                      .build();

if (transaction.isValid()) {
    // Execute the transaction
    manager->executeTransaction(transaction);
} else {
    qDebug() << "Invalid transaction:" << transaction.validationError();
}
```

### Create New Entry

```cpp
auto newEntryId = QUuid::createUuid();
auto parentGroupId = QUuid::createUuid();

auto transaction = TransactionBuilder::forEntry(newEntryId)
                      .withDescription("Create email account")
                      .createEntry(parentGroupId)
                      .withTitle("Email Account")
                      .withUsername("user@example.com")
                      .withPassword("initialPassword456!")
                      .build();
```

### Group Operations

```cpp
auto groupId = QUuid::createUuid();

auto transaction = TransactionBuilder::forGroup(groupId)
                      .withDescription("Update group properties")
                      .updateGroup()
                      .withName("Work Accounts")
                      .withIcon(5)
                      .withGroupProperty("IsExpanded", true)
                      .build();
```

### Database Operations

```cpp
auto transaction = TransactionBuilder::forDatabase()
                      .withDescription("Update database metadata")
                      .updateDatabase()
                      .withDatabaseProperty("Name", "My Passwords")
                      .withDatabaseProperty("Description", "Personal database")
                      .build();
```

## Factory Methods

Start building transactions with these static factory methods:

| Method | Purpose |
|--------|---------|
| `TransactionBuilder::forEntry(entryId)` | Build entry-related transactions |
| `TransactionBuilder::forGroup(groupId)` | Build group-related transactions |
| `TransactionBuilder::forDatabase()` | Build database-related transactions |

## Entry Operations

### Operations
- `.createEntry(parentGroupId)` - Create new entry in specified group
- `.updateEntry()` - Update existing entry
- `.deleteEntry()` - Delete entry
- `.moveEntryTo(targetGroupId)` - Move entry to different group

### Attribute Methods
- `.withTitle(title)` - Set entry title
- `.withUsername(username)` - Set username
- `.withPassword(password)` - Set password (automatically protected)
- `.withUrl(url)` - Set URL
- `.withNotes(notes)` - Set notes
- `.withAttribute(key, value, isProtected)` - Set custom attribute
- `.withProperty(property, value)` - Set entry property

## Group Operations

### Operations
- `.createGroup(parentGroupId)` - Create new group in specified parent
- `.updateGroup()` - Update existing group
- `.deleteGroup()` - Delete group
- `.moveGroupTo(targetParentId)` - Move group to different parent

### Property Methods
- `.withName(name)` - Set group name
- `.withIcon(iconIndex)` - Set group icon
- `.withGroupProperty(property, value)` - Set custom group property

## Database Operations

### Operations
- `.updateDatabase()` - Update database properties

### Property Methods
- `.withDatabaseProperty(property, value)` - Set database property

## Validation

The builder validates transactions during construction and provides detailed error messages:

```cpp
auto builder = TransactionBuilder::forEntry(entryId)
                  .withDescription("Invalid transaction")
                  .withTitle("Some title"); // Missing operation

if (!builder.isValid()) {
    qDebug() << "Error:" << builder.validationError();
    // Output: "No operation specified. Use create(), update(), delete(), or move methods"
}
```

### Common Validation Errors

- **No operation specified**: Must call one operation method (create, update, delete, move)
- **No changes for update**: Update operations require at least one change
- **Missing target ID**: Create and move operations require target IDs
- **Mixed transaction types**: Cannot mix entry and group operations

## Error Handling

The builder is designed for non-exception environments. Instead of throwing exceptions, it:

1. **Ignores invalid operations**: Calls to incompatible methods are silently ignored
2. **Validates on build**: Use `isValid()` and `validationError()` to check results
3. **Returns invalid transactions**: Invalid transactions can still be built but will fail validation

## Best Practices

### 1. Always Validate

```cpp
auto transaction = TransactionBuilder::forEntry(entryId)
                      .updateEntry()
                      .withTitle("New Title")
                      .build();

if (transaction.isValid()) {
    manager->executeTransaction(transaction);
} else {
    qDebug() << "Validation failed:" << transaction.validationError();
}
```

### 2. Use Descriptive Messages

```cpp
auto transaction = TransactionBuilder::forEntry(entryId)
                      .withDescription("Update password after security audit")
                      .updateEntry()
                      .withPassword("newSecurePassword123!")
                      .build();
```

### 3. Group Related Changes

```cpp
// Good: Single transaction for related changes
auto transaction = TransactionBuilder::forEntry(entryId)
                      .updateEntry()
                      .withUsername("new.email@company.com")
                      .withUrl("https://company.com/login")
                      .withNotes("Updated after company migration")
                      .build();
```

## Architecture Benefits

### Type Safety
- Compile-time validation of transaction structure
- Prevents runtime errors from invalid combinations

### Error Prevention
- Cannot mix entry and group operations
- Cannot create update transactions without changes
- Cannot forget required target IDs

### Readability
- Fluent interface makes transaction intent clear
- Self-documenting code through method names

### Maintainability
- Centralized transaction construction logic
- Easy to add new validation rules
- Consistent error handling

## Integration with DirectTransactionManager

```cpp
#include "TransactionBuilder.h"
#include "DirectTransactionManager.h"

// Assuming you have a database instance
DirectTransactionManager manager(database);

auto transaction = TransactionBuilder::forEntry(entryId)
                      .withDescription("Automated password update")
                      .updateEntry()
                      .withPassword(generateSecurePassword())
                      .build();

if (transaction.isValid()) {
    bool success = manager.executeTransaction(transaction);
    if (success) {
        qDebug() << "Transaction executed successfully";
    }
}
```

## Testing

The TransactionBuilder includes comprehensive unit tests covering:

- Valid transaction construction
- Validation error detection
- Edge cases and error conditions
- Integration with transaction types

Run tests with:
```bash
cd build && ninja src/transactional/TransactionalTest
QT_QPA_PLATFORM=minimal ./src/transactional/TransactionalTest
```

## Demo

See the complete working examples:
```bash
cd build && ninja src/transactional/TransactionBuilderDemo
QT_QPA_PLATFORM=minimal ./src/transactional/TransactionBuilderDemo
```

This demonstrates all features including error handling and validation.