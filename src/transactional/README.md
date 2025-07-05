# KeePassXC Transactional Architecture Implementation

This directory contains a proof-of-concept implementation of the transactional architecture proposed for KeePassXC. This is a temporary experimental module designed to test and validate the proposed approach before integrating into the main codebase.

## Overview

The transactional architecture aims to:

- **Replace mutable data structures** with immutable copy-on-write objects
- **Enable comprehensive undo/redo** functionality throughout the application
- **Provide audit trails** for all database modifications
- **Improve separation** between GUI logic and data manipulation
- **Enable bulk operations** and scripting capabilities

## Key Components

### 1. `Transaction.h/cpp`
JSON-based transaction format that represents all possible database modifications:

```cpp
Transaction transaction(TransactionType::UpdateEntry, "Update GitHub password");
transaction.setEntryTarget(entryId);
transaction.setAttributeChange("Password", "NewPassword123", true);
```

### 2. `TransactionManager.h/cpp`
Manages transaction execution, undo/redo history, and audit trails:

```cpp
TransactionManager manager(database);
manager.executeTransaction(transaction);
manager.undo(); // Reverses the last operation
manager.redo(); // Re-applies the undone operation
```

### 3. `ImmutableEntry.h/cpp`
Copy-on-write immutable Entry class demonstrating the new data structure pattern:

```cpp
ImmutableEntry entry;
ImmutableEntry updated = entry.withTitle("New Title")
                             .withPassword("NewPassword123")
                             .withAttribute("Notes", "Updated", false);
```

## Usage Examples

### Basic Entry Modification
```cpp
// Old approach (direct mutation)
entry->setTitle("New Title");
entry->setPassword("NewPassword");

// New approach (immutable + transaction)
auto newEntry = entry.withTitle("New Title").withPassword("NewPassword");
Transaction tx(TransactionType::UpdateEntry, "Update credentials");
tx.setEntryTarget(entry.uuid());
tx.setAttributeChange("Title", "New Title", false);
tx.setAttributeChange("Password", "NewPassword", true);
manager.executeTransaction(tx);
```

### Batch Operations
```cpp
manager.beginBatch("Import from CSV");
for (const auto& csvRow : csvData) {
    Transaction tx(TransactionType::CreateEntry, "Import entry");
    tx.setAttributeChange("Title", csvRow.title, false);
    tx.setAttributeChange("Password", csvRow.password, true);
    manager.executeTransaction(tx);
}
manager.endBatch(); // Single undo operation for entire import
```

### Undo/Redo
```cpp
// Make some changes
manager.executeTransaction(createTransaction);
manager.executeTransaction(updateTransaction);

// Undo last change
if (manager.canUndo()) {
    manager.undo(); // Reverses updateTransaction
}

// Redo if needed
if (manager.canRedo()) {
    manager.redo(); // Re-applies updateTransaction
}
```

## Building and Testing

### Prerequisites
This module requires Qt5 and follows the same build requirements as the main KeePassXC project.

### Building
From the main KeePassXC build directory:

```bash
# Configure with tests enabled
cmake -G Ninja -DWITH_XC_ALL=ON -DWITH_GUI_TESTS=ON ..

# Build the transactional module
cmake --build . --target transactional

# Build and run tests
cmake --build . --target TransactionalTest
./src/transactional/TransactionalTest

# Build and run demo
cmake --build . --target TransactionalDemo
./src/transactional/TransactionalDemo
```

### Running Tests
```bash
# Run the unit tests
ctest -R TransactionalTest -V

# Or run directly
./src/transactional/TransactionalTest
```

### Running Demo
```bash
# Run the demonstration program
./src/transactional/TransactionalDemo
```

The demo will show:
- Immutable entry creation and modification
- Transaction creation and serialization
- Copy-on-write behavior
- Usage patterns and best practices

## JSON Transaction Format

Transactions are serialized to JSON for maximum portability and tooling support:

```json
{
    "id": "550e8400-e29b-41d4-a716-446655440000",
    "type": "UpdateEntry",
    "timestamp": "2024-01-15T10:30:00Z",
    "description": "Update password for GitHub account",
    "target": {
        "entryId": "entry-uuid-here"
    },
    "changes": {
        "attributes": {
            "Password": {
                "value": "newPassword",
                "protected": true
            }
        },
        "properties": {
            "modified": "2024-01-15T10:30:00Z"
        }
    }
}
```

## Architecture Benefits

### Immediate Benefits
- **Complete Undo/Redo**: Every operation can be reversed
- **Audit Trail**: Full history of all changes with timestamps
- **Atomic Operations**: Changes are all-or-nothing
- **Thread Safety**: Immutable objects are inherently thread-safe

### Future Capabilities
- **Collaboration**: Conflict resolution for team databases
- **Scripting**: JSON transactions enable automation
- **Bulk Operations**: Efficient multi-entry modifications
- **Change Tracking**: Detailed diff capabilities

## Integration Strategy

This experimental module demonstrates the core concepts. Integration into the main codebase would involve:

1. **Phase 1**: Add transaction layer alongside existing mutable classes
2. **Phase 2**: Gradually migrate GUI to use transactions
3. **Phase 3**: Replace mutable data structures with immutable ones
4. **Phase 4**: Remove legacy mutation methods

## Backward Compatibility

The design maintains backward compatibility by:
- Keeping existing classes unchanged initially
- Adding transaction layer as an optional interface
- Providing compatibility wrappers for legacy code
- Migrating incrementally rather than wholesale replacement

## Performance Considerations

- **Copy-on-Write**: Minimal memory overhead for unmodified objects
- **Shared Data**: Qt's implicit sharing reduces memory usage
- **Lazy Evaluation**: Transactions can be batched and optimized
- **JSON Overhead**: Acceptable for the flexibility gained

## Current Limitations

This is a proof-of-concept implementation with several limitations:

1. **Database Integration**: Not connected to actual Database class
2. **Full Transaction Types**: Only basic transaction types implemented
3. **Persistence**: Transaction history not saved to disk
4. **GUI Integration**: No UI components included
5. **Performance**: Not optimized for production use

## Next Steps

1. **Team Review**: Discuss approach with development team
2. **Database Integration**: Connect with actual Database/Entry/Group classes
3. **GUI Prototype**: Create simple undo/redo UI
4. **Performance Testing**: Measure memory and CPU impact
5. **Migration Planning**: Define incremental integration strategy

## Files Overview

- `Transaction.h/cpp` - JSON-based transaction representation
- `TransactionManager.h/cpp` - Transaction execution and undo/redo
- `ImmutableEntry.h/cpp` - Copy-on-write immutable Entry class
- `TransactionalTest.cpp` - Unit tests demonstrating functionality
- `TransactionalDemo.cpp` - Interactive demonstration program
- `CMakeLists.txt` - Build configuration
- `README.md` - This documentation

This implementation provides a solid foundation for discussing and refining the transactional architecture approach for KeePassXC.