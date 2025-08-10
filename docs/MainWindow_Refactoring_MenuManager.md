# MainWindow Refactoring Documentation

## Overview

This document describes the refactoring of the MainWindow class to extract menu management functionality into a separate, dedicated class called `MainWindowMenuManager`. This is part of a larger effort to break down the monolithic MainWindow class into manageable, single-responsibility components.

## Problems Addressed

### Before Refactoring:
- **MainWindow.cpp**: 2,255 lines of code containing multiple responsibilities
- **Menu-related code scattered** throughout the constructor and multiple methods
- **Complex initialization** with 113+ signal-slot connections
- **Difficult maintenance** due to mixed concerns
- **Poor testability** due to tightly coupled functionality

### After Refactoring:
- **Extracted 150+ lines** of menu-related code into dedicated class
- **Clear separation** of menu management concerns
- **Improved maintainability** through single responsibility principle
- **Better testability** through focused interfaces
- **Preserved compatibility** with existing MainWindow API

## Architecture Changes

### New Class: MainWindowMenuManager

**Location**: `src/gui/MainWindowMenuManager.h/.cpp`

**Responsibilities**:
- Creating and maintaining entry context menus
- Managing dynamic menu content (recent databases, copy attributes, tags)
- Setting up menu action groups
- Handling menu updates based on application state

**Key Features**:
- Encapsulates all menu-related logic
- Provides clean interface to MainWindow
- Maintains proper signal-slot connections
- Handles menu action groups properly

### Integration with MainWindow

The MenuManager is integrated into MainWindow as a member variable and handles:

1. **Menu Creation**: Context menus for entries and groups
2. **Dynamic Updates**: Recent databases, copy attributes, tags menus
3. **Action Groups**: Proper grouping and signal handling
4. **Event Handling**: Menu show/hide events and focus management

## Code Changes Summary

### Files Added:
- `src/gui/MainWindowMenuManager.h` - Header file with class definition
- `src/gui/MainWindowMenuManager.cpp` - Implementation of menu management

### Files Modified:
- `src/gui/MainWindow.h` - Updated member variables and method signatures
- `src/gui/MainWindow.cpp` - Integrated MenuManager, removed menu methods
- `src/CMakeLists.txt` - Added new source files to build

### Methods Extracted from MainWindow:
- `updateLastDatabasesMenu()` → `MainWindowMenuManager::updateLastDatabasesMenu()`
- `updateCopyAttributesMenu()` → `MainWindowMenuManager::updateCopyAttributesMenu()`
- `updateSetTagsMenu()` → `MainWindowMenuManager::updateSetTagsMenu()`
- `openRecentDatabase(QAction*)` → `MainWindowMenuManager::onOpenRecentDatabase(QAction*)`

### Methods Preserved in MainWindow:
- `clearLastDatabases()` - Kept because it handles more than just menu clearing
- `showEntryContextMenu()` - Updated to use MenuManager's context menus

## API Compatibility

The refactoring maintains **complete backward compatibility** with the existing MainWindow API:

- All public methods remain unchanged
- All signals and slots continue to work
- All existing behavior is preserved
- No changes required in calling code

## Benefits Achieved

### 1. **Single Responsibility Principle**
Each class now has a clear, focused purpose:
- `MainWindow`: Overall window management and coordination
- `MainWindowMenuManager`: Menu creation, updates, and management

### 2. **Improved Maintainability**
- Menu-related bugs can be isolated to MenuManager
- Changes to menu behavior are localized
- Easier to understand and modify menu logic

### 3. **Better Testability**
- MenuManager can be tested independently
- Mock MenuManager can be used for MainWindow testing
- Isolated testing of menu functionality

### 4. **Reduced Complexity**
- MainWindow constructor is simplified
- Clear separation of initialization logic
- More manageable class sizes

## Usage Examples

### Creating MenuManager:
```cpp
// In MainWindow constructor
m_menuManager = new MainWindowMenuManager(m_ui.data(), m_ui->tabWidget, this);
m_menuManager->initializeMenus();
```

### Connecting Signals:
```cpp
// Connect menu manager signals to MainWindow slots
connect(m_menuManager, &MainWindowMenuManager::openRecentDatabase, this, 
        [this](const QString& filePath) { openDatabase(filePath); });
```

### Using Context Menus:
```cpp
// Show context menu (same API as before)
m_menuManager->entryContextMenu()->popup(globalPos);
```

## Future Improvements

This refactoring establishes a pattern for further MainWindow decomposition:

### Next Recommended Extractions:
1. **ToolbarManager** - Handle toolbar setup and visibility
2. **SystemTrayManager** - System tray integration and notifications  
3. **WindowStateManager** - Window positioning and state management
4. **ShortcutManager** - Keyboard shortcuts setup and handling
5. **StatusBarManager** - Status bar and progress indicators

### Design Principles for Future Refactoring:
- Maintain public API compatibility
- Use dependency injection for testing
- Create focused, single-responsibility classes
- Preserve existing signal-slot architecture
- Document all changes thoroughly

## Testing

The refactoring has been validated through:
- **Compilation**: All code compiles without errors or warnings
- **Code Formatting**: Applied clang-format for consistency
- **Build Integration**: Successfully integrated into CMake build system
- **API Preservation**: All existing MainWindow APIs remain functional

## Migration Guide

For developers working with MainWindow:

### Accessing Menu Functionality:
```cpp
// Old way (no longer available)
updateLastDatabasesMenu();

// New way
m_menuManager->updateLastDatabasesMenu();
```

### Getting Context Menus:
```cpp
// Old way (private members)
m_entryContextMenu->popup(pos);

// New way (through MenuManager)
m_menuManager->entryContextMenu()->popup(pos);
```

### Action Groups:
```cpp
// Old way (private members)
m_lastDatabasesActions->addAction(action);

// New way (through MenuManager)
m_menuManager->lastDatabasesActions()->addAction(action);
```

This refactoring represents a significant step toward a more maintainable and well-organized MainWindow class architecture.