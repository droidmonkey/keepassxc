# KeePassXC Transactional Architecture Implementation

This directory contains a proof-of-concept implementation of the transactional architecture proposed for KeePassXC. This is a temporary experimental module designed to test and validate the proposed approach before integrating into the main codebase.

## Overview

The transactional architecture aims to:

- **Replace mutable data structures** with immutable copy-on-write objects
- **Enable comprehensive undo/redo** functionality throughout the application
- **Provide audit trails** for all database modifications
- **Improve separation** between GUI logic and data manipulation
- **Enable bulk operations** and scripting capabilities

## Two Implementation Approaches

This module provides two different approaches to transaction management:

### 1. JSON-Based Transactions (`Transaction.h/cpp`, `TransactionManager.h/cpp`)
Uses JSON serialization for maximum flexibility and external tool support:

```cpp
Transaction transaction(TransactionType::UpdateEntry, "Update GitHub password");
transaction.setEntryTarget(entryId);
transaction.setAttributeChange("Password", "NewPassword123", true);
```

**Advantages:**
- Flexible key-value storage
- Easy serialization for persistence/networking
- Dynamic property access
- Great for scripting and external tools

**Disadvantages:**
- Runtime overhead from JSON processing
- Runtime validation only
- More memory usage

### 2. Direct Code Transactions (`DirectTransaction.h/cpp`, `DirectTransactionManager.h/cpp`)
Uses type-safe C++ structures for maximum performance and compile-time validation:

```cpp
DirectTransaction transaction(DirectTransactionType::UpdateEntry, "Update GitHub password");
transaction.setEntryTarget(entryId);
transaction.setEntryAttribute("Password", "NewPassword123", true);
```

**Advantages:**
- Faster execution (no JSON parsing)
- Lower memory overhead
- Type safety at compile time
- Direct member access

**Disadvantages:**
- Less flexible for dynamic use cases
- Harder to serialize for external tools
- More rigid structure

## Key Components

### JSON-Based Components

#### `Transaction.h/cpp`
JSON-based transaction format that represents all possible database modifications using QJsonObject internally.

#### `TransactionManager.h/cpp`
Manages JSON transaction execution, undo/redo history, and audit trails.

### Direct Code Components

#### `DirectTransaction.h/cpp`
Type-safe transaction format using C++ structs for targets and changes.

#### `DirectTransactionManager.h/cpp`
Manages direct transaction execution with typed structures and convenience methods.

### Shared Components

#### `ImmutableEntry.h/cpp`
Copy-on-write immutable Entry class demonstrating the new data structure pattern:

```cpp
ImmutableEntry entry;
ImmutableEntry updated = entry.withTitle("New Title")
                             .withPassword("NewPassword123")
                             .withAttribute("Notes", "Updated", false);
```

## Usage Examples

### JSON Approach
```cpp
// Create transaction
Transaction tx(TransactionType::UpdateEntry, "Update credentials");
tx.setEntryTarget(entry.uuid());
tx.setAttributeChange("Title", "New Title", false);
tx.setAttributeChange("Password", "NewPassword", true);
manager.executeTransaction(tx);
```

### Direct Approach
```cpp
// Create transaction
DirectTransaction tx(DirectTransactionType::UpdateEntry, "Update credentials");
tx.setEntryTarget(entry.uuid());
tx.setEntryAttribute("Title", "New Title", false);
tx.setEntryAttribute("Password", "NewPassword", true);
directManager.executeTransaction(tx);

// Or use convenience methods
directManager.updateEntryTitle(entryId, "New Title");
directManager.updateEntryPassword(entryId, "NewPassword");
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

# Build and run JSON demo
cmake --build . --target TransactionalDemo
./src/transactional/TransactionalDemo

# Build and run Direct demo
cmake --build . --target DirectTransactionDemo
./src/transactional/DirectTransactionDemo
```

### Running Tests
```bash
# Run the unit tests
ctest -R TransactionalTest -V

# Or run directly
./src/transactional/TransactionalTest
```

### Running Demos
```bash
# Run the JSON-based demonstration program
./src/transactional/TransactionalDemo

# Run the direct transaction demonstration program
./src/transactional/DirectTransactionDemo
```

The demos will show:
- Immutable entry creation and modification
- Transaction creation and execution
- Copy-on-write behavior
- Undo/redo functionality
- Usage patterns and best practices

## JSON Transaction Format

JSON transactions are serialized for maximum portability and tooling support:

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

## Choosing the Right Approach

**Use JSON-based transactions when:**
- Building external tools or scripts
- Need dynamic property handling
- Serialization for network/storage is important
- Maximum flexibility is required

**Use Direct transactions when:**
- Performance is critical
- Type safety is important
- Working within C++ codebase
- Memory efficiency is a concern

## Performance Considerations

- **Copy-on-Write**: Minimal memory overhead for unmodified objects
- **Shared Data**: Qt's implicit sharing reduces memory usage
- **Lazy Evaluation**: Transactions can be batched and optimized
- **JSON Overhead**: Acceptable for the flexibility gained (JSON approach)
- **Type Safety**: Compile-time validation prevents runtime errors (Direct approach)

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

### JSON-Based Implementation
- `Transaction.h/cpp` - JSON-based transaction representation
- `TransactionManager.h/cpp` - JSON transaction execution and undo/redo
- `TransactionalTest.cpp` - Unit tests demonstrating JSON functionality
- `TransactionalDemo.cpp` - Interactive JSON demonstration program

### Direct Implementation
- `DirectTransaction.h/cpp` - Type-safe transaction representation
- `DirectTransactionManager.h/cpp` - Direct transaction execution with convenience methods
- `DirectTransactionDemo.cpp` - Interactive direct transaction demonstration program

### Shared Components
- `ImmutableEntry.h/cpp` - Copy-on-write immutable Entry class
- `CMakeLists.txt` - Build configuration
- `README.md` - This documentation

This implementation provides both flexible (JSON) and performant (Direct) approaches to transaction management, allowing the team to choose the best fit for different use cases within KeePassXC.