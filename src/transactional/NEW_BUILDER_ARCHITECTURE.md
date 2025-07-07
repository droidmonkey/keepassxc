# New Transaction Builder Architecture

This document describes the improved transaction builder architecture that provides type-safe, cleaner transaction construction for KeePassXC database operations.

## Overview

The new architecture addresses the feedback to create a cleaner delineation between group and entry transactions while reducing the number of function calls and preventing programming mistakes.

## Key Improvements

### 1. Type Safety
- **Compile-time prevention** of mixing entry and group operations
- **Separate builder classes** for entries and groups
- **Cannot accidentally** call `withTitle()` on a group builder

### 2. Cleaner API
- **Fewer function calls** through bulk operations
- **Auto-UUID generation** for create operations
- **Fluent interface** with method chaining

### 3. Better Organization
- **Base class** with common functionality
- **Specialized builders** for specific types
- **Factory methods** that return the correct builder type

## Architecture

```
TransactionBuilderBase
├── Common methods (withDescription, withIcon, withCustomData)
├── Validation and build logic
└── Virtual methods for type-specific operations

EntryTransactionBuilder : TransactionBuilderBase
├── Entry-specific methods (withTitle, withPassword, etc.)
├── Bulk attribute operations
└── Entry validation logic

GroupTransactionBuilder : TransactionBuilderBase
├── Group-specific methods (withName, withAutoTypeEnabled, etc.)
├── Bulk property operations
└── Group validation logic

Builder (Static Factory)
├── Entry operations (createEntry, updateEntry, etc.)
└── Group operations (createGroup, updateGroup, etc.)
```

## Usage Examples

### Entry Operations

```cpp
// Create entry with auto-generated UUID
auto transaction = Builder::createEntry(parentGroupId)
    .withDescription("Create GitHub login")
    .withTitle("GitHub Account")
    .withUsername("john.doe@example.com")
    .withPassword("SecurePassword123!")
    .withUrl("https://github.com")
    .withTags({"work", "development"})
    .withIcon(1)
    .build();

// Update entry with bulk attributes
QMap<QString, QString> customAttribs;
customAttribs["Department"] = "Engineering";
customAttribs["Employee ID"] = "ENG-001";

QMap<QString, QString> protectedAttribs;
protectedAttribs["API Key"] = "secret-key-12345";

auto transaction = Builder::updateEntry(entryId)
    .withDescription("Update with bulk attributes")
    .withAttributes(customAttribs)
    .withProtectedAttributes(protectedAttribs)
    .withForegroundColor("#FF0000")
    .build();

// Move entry to different group
auto transaction = Builder::moveEntry(entryId)
    .withDescription("Move to archive")
    .toGroup(targetGroupId)
    .build();
```

### Group Operations

```cpp
// Create group with auto-generated UUID
auto transaction = Builder::createGroup(parentGroupId)
    .withDescription("Create project folder")
    .withName("Web Development")
    .withNotes("Web development credentials")
    .withExpanded(true)
    .withAutoTypeEnabled(GroupTransactionBuilder::TriState::Enable)
    .withIcon(2)
    .build();

// Update group with bulk properties
QMap<QString, QVariant> properties;
properties["tags"] = "project,development";
properties["defaultAutoTypeSequence"] = "{USERNAME}{TAB}{PASSWORD}";

auto transaction = Builder::updateGroup(groupId)
    .withDescription("Update configuration")
    .withProperties(properties)
    .withMergeMode(GroupTransactionBuilder::MergeMode::Synchronize)
    .build();

// Move group to different parent
auto transaction = Builder::moveGroup(groupId)
    .withDescription("Reorganize structure")
    .toParent(newParentId)
    .build();
```

### Bulk Operations

```cpp
// Replace all attributes at once
QMap<QString, QString> newAttribs;
newAttribs["Title"] = "New Title";
newAttribs["UserName"] = "newuser";
newAttribs["Password"] = "newpass";

auto transaction = Builder::updateEntry(entryId)
    .replaceAllAttributes(newAttribs, {"Password"}) // Password is protected
    .build();

// Clear specific attributes
auto transaction = Builder::updateEntry(entryId)
    .clearAttributes({"Old Field", "Deprecated"})
    .withTitle("Updated Title")
    .build();

// Clear all custom attributes
auto transaction = Builder::updateEntry(entryId)
    .clearAllCustomAttributes()
    .withTitle("Clean Entry")
    .build();
```

## Key Features

### Auto-UUID Generation
Create operations automatically generate UUIDs if not provided:
```cpp
// Auto-generates UUID
auto transaction = Builder::createEntry(parentGroupId)
    .withTitle("New Entry")
    .build();

// Use specific UUID
auto transaction = Builder::createEntry(parentGroupId, specificUuid)
    .withTitle("New Entry")
    .build();
```

### Comprehensive Validation
- **Compile-time**: Cannot mix entry and group operations
- **Build-time**: Validates required fields and logical consistency
- **Runtime**: Clear error messages for invalid operations

### Type Safety Examples
```cpp
// This compiles - correct types
auto entryBuilder = Builder::updateEntry(entryId);
entryBuilder.withTitle("Title");       // ✅ Entry method
entryBuilder.withPassword("Password"); // ✅ Entry method

auto groupBuilder = Builder::updateGroup(groupId);
groupBuilder.withName("Name");         // ✅ Group method
groupBuilder.withExpanded(true);       // ✅ Group method

// This would NOT compile - mixing types
// entryBuilder.withName("Name");      // ❌ Group method on entry builder
// groupBuilder.withTitle("Title");    // ❌ Entry method on group builder
```

### Error Handling
```cpp
// Validation before build
auto builder = Builder::updateEntry(entryId);
if (!builder.isValid()) {
    qDebug() << "Error:" << builder.validationError();
    return;
}

// Exception on invalid build
try {
    auto transaction = Builder::updateEntry(entryId).build(); // No changes
} catch (const std::exception& e) {
    qDebug() << "Build failed:" << e.what();
}
```

## Benefits

### For Developers
- **Fewer mistakes**: Type system prevents errors
- **Cleaner code**: Reduced boilerplate and clearer intent
- **Better tools**: IDE autocompletion works better
- **Easier testing**: Each builder can be tested independently

### For Users
- **More reliable**: Fewer bugs from developer mistakes
- **Better performance**: Bulk operations are more efficient
- **Consistent behavior**: Standardized transaction patterns

### For the Codebase
- **Maintainable**: Clear separation of concerns
- **Extensible**: Easy to add new transaction types
- **Testable**: Comprehensive test coverage possible
- **Documented**: Self-documenting API design

## Migration from Old Builder

The old TransactionBuilder is still available for backward compatibility, but new code should use the new architecture:

```cpp
// Old approach (still works)
auto transaction = TransactionBuilder::forEntry(entryId)
    .updateEntry()
    .withTitle("Title")
    .build();

// New approach (recommended)
auto transaction = Builder::updateEntry(entryId)
    .withTitle("Title")
    .build();
```

## Testing

The new architecture includes comprehensive tests:
- Unit tests for each builder type
- Integration tests for complex operations
- Validation tests for error conditions
- Performance tests for bulk operations

Run tests with:
```bash
cd build && ctest -R NewBuilderTest
```

## Future Enhancements

This architecture enables future improvements:
- **Batch transactions**: Multiple operations in one transaction
- **Conditional operations**: Operations based on current state
- **Templating**: Pre-configured transaction templates
- **Scripting**: Script-friendly transaction construction
- **Async operations**: Non-blocking transaction execution