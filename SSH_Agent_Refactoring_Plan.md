# SSH Agent Refactoring Detailed Plan

## Current SSH Agent Implementation Analysis

### Existing Code Structure

The SSH Agent functionality is currently embedded within EditEntryWidget with the following methods:

```cpp
// Setup and initialization
void setupSSHAgent();                    // Line 588 - UI setup and connections
void setSSHAgentSettings();             // Line 621 - Apply settings to UI

// Data management  
void updateSSHAgent();                  // Line 634 - Load entry data
void updateSSHAgentAttachment();        // Line 649 - Handle attachment selection
void updateSSHAgentAttachments();       // Line 655 - Populate attachment list
void updateSSHAgentKeyInfo();           // Line 685 - Display key information

// User actions
void browsePrivateKey();                // Browse for external key file
void addKeyToAgent();                   // Add key to SSH agent
void removeKeyFromAgent();              // Remove key from SSH agent  
void clearAgent();                      // Clear all keys from agent
void decryptPrivateKey();               // Decrypt encrypted key
void copyPublicKey();                   // Copy public key to clipboard
void generatePrivateKey();              // Generate new key pair

// Utility
bool getOpenSSHKey(OpenSSHKey& key, bool decrypt = false);  // Extract key data
void toKeeAgentSettings(KeeAgentSettings& settings) const;  // Convert to settings
```

### Problems with Current Implementation

1. **Mixed Responsibilities**: UI management, file I/O, cryptographic operations, and agent communication all in one class
2. **Complex State Management**: Multiple UI controls with interdependent state
3. **Poor Testability**: Difficult to test SSH functionality without full widget
4. **Tight Coupling**: Direct dependencies on UI elements throughout business logic

## Proposed SSH Agent Architecture

### Component Overview

```
SSHAgentPageController (coordinator)
├── SSHAgentEditWidget (UI management)
├── SSHKeyDataModel (data and validation)
├── SSHAgentService (agent communication)
└── SSHKeyFileHandler (file operations)
```

### Detailed Component Design

#### 1. SSHAgentPageController

**File**: `src/gui/entry/sshagent/SSHAgentPageController.h`

```cpp
#ifndef SSHAGENTPAGECONTROLLER_H
#define SSHAGENTPAGECONTROLLER_H

#include "gui/entry/EntryPageController.h"
#include "sshagent/KeeAgentSettings.h"

class SSHAgentEditWidget;
class SSHKeyDataModel;
class SSHAgentService;
class Entry;

class SSHAgentPageController : public EntryPageController
{
    Q_OBJECT

public:
    explicit SSHAgentPageController(QWidget* parent = nullptr);
    ~SSHAgentPageController() override;

    // EntryPageController interface
    void loadEntry(Entry* entry) override;
    void saveEntry(Entry* entry) override;
    bool validateData() override;
    QWidget* widget() override;
    void setModified(bool modified) override;

public slots:
    void addKeyToAgent();
    void removeKeyFromAgent();
    void clearAgent();
    void generatePrivateKey();
    void copyPublicKey();

private slots:
    void onKeyDataChanged();
    void onSettingsChanged();

private:
    void connectSignals();
    void updateAgentButtons();

    SSHAgentEditWidget* m_widget;
    SSHKeyDataModel* m_keyModel;
    SSHAgentService* m_agentService;
    Entry* m_entry;
    bool m_modified;
};

#endif // SSHAGENTPAGECONTROLLER_H
```

#### 2. SSHAgentEditWidget

**File**: `src/gui/entry/sshagent/SSHAgentEditWidget.h`

```cpp
#ifndef SSHAGENTEDITORWIDGET_H
#define SSHAGENTEDITORWIDGET_H

#include <QWidget>

class SSHKeyDataModel;
class KeeAgentSettings;

namespace Ui {
class EditEntryWidgetSSHAgent;
}

class SSHAgentEditWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SSHAgentEditWidget(QWidget* parent = nullptr);
    ~SSHAgentEditWidget();

    void setKeyModel(SSHKeyDataModel* model);
    void setSettings(const KeeAgentSettings& settings);
    KeeAgentSettings getSettings() const;

signals:
    void settingsChanged();
    void browsePrivateKeyRequested();
    void addKeyToAgentRequested();
    void removeKeyFromAgentRequested();
    void clearAgentRequested();
    void decryptPrivateKeyRequested();
    void copyPublicKeyRequested();
    void generatePrivateKeyRequested();

private slots:
    void onKeyDataChanged();
    void onAttachmentSelectionChanged();
    void onExternalFileChanged();

private:
    void setupUI();
    void connectSignals();
    void updateKeyInfo();
    void updateAttachmentList();

    Ui::EditEntryWidgetSSHAgent* m_ui;
    SSHKeyDataModel* m_keyModel;
};

#endif // SSHAGENTEDITORWIDGET_H
```

#### 3. SSHKeyDataModel

**File**: `src/gui/entry/sshagent/SSHKeyDataModel.h`

```cpp
#ifndef SSHKEYDATAMODEL_H
#define SSHKEYDATAMODEL_H

#include <QObject>
#include <QByteArray>

class Entry;
class EntryAttachments;
class OpenSSHKey;

class SSHKeyDataModel : public QObject
{
    Q_OBJECT

public:
    enum KeySource {
        NoKey,
        Attachment,
        ExternalFile
    };

    explicit SSHKeyDataModel(QObject* parent = nullptr);

    // Key source management
    void setKeySource(KeySource source);
    KeySource keySource() const;

    void setAttachmentName(const QString& name);
    QString attachmentName() const;

    void setExternalFilePath(const QString& path);
    QString externalFilePath() const;

    // Key information
    bool isValid() const;
    bool isEncrypted() const;
    QString fingerprint(QCryptographicHash::Algorithm algorithm = QCryptographicHash::Sha256) const;
    QString comment() const;
    QString publicKey() const;
    
    // Key operations
    bool loadKey();
    bool decrypt(const QString& passphrase);
    QByteArray keyData() const;

    // Entry integration
    void setEntry(Entry* entry);
    void setAttachments(EntryAttachments* attachments);

signals:
    void keyDataChanged();
    void validationStateChanged(bool valid);
    void keySourceChanged(KeySource source);

private slots:
    void onAttachmentsModified();

private:
    bool loadFromAttachment();
    bool loadFromFile();
    void updateKeyInfo();
    void clearKeyData();

    Entry* m_entry;
    EntryAttachments* m_attachments;
    KeySource m_keySource;
    QString m_attachmentName;
    QString m_externalFilePath;
    
    std::unique_ptr<OpenSSHKey> m_key;
    bool m_valid;
};

#endif // SSHKEYDATAMODEL_H
```

#### 4. SSHAgentService

**File**: `src/gui/entry/sshagent/SSHAgentService.h`

```cpp
#ifndef SSHAGENTSERVICE_H  
#define SSHAGENTSERVICE_H

#include <QObject>

class OpenSSHKey;
class KeeAgentSettings;

class SSHAgentService : public QObject
{
    Q_OBJECT

public:
    explicit SSHAgentService(QObject* parent = nullptr);

    bool isAgentAvailable() const;
    bool addKey(const OpenSSHKey& key, const KeeAgentSettings& settings);
    bool removeKey(const OpenSSHKey& key);
    bool clearAgent();
    
    QStringList listKeys() const;
    bool isKeyLoaded(const OpenSSHKey& key) const;

signals:
    void keyAdded(const QString& fingerprint);
    void keyRemoved(const QString& fingerprint);
    void agentCleared();
    void error(const QString& message);

private:
    bool connectToAgent();
    QByteArray buildConstraints(const KeeAgentSettings& settings);
};

#endif // SSHAGENTSERVICE_H
```

## Migration Strategy

### Step 1: Create Base Infrastructure
1. Create directory structure: `src/gui/entry/sshagent/`
2. Implement `EntryPageController` base interface
3. Set up CMake build configuration

### Step 2: Extract Data Model
1. Implement `SSHKeyDataModel` with current key loading logic
2. Create unit tests for key data model
3. Migrate key validation and information display logic

### Step 3: Extract Service Layer
1. Implement `SSHAgentService` with agent communication logic  
2. Extract agent operations from EditEntryWidget
3. Add error handling and async operation support

### Step 4: Create UI Component
1. Implement `SSHAgentEditWidget` using existing .ui file
2. Move UI setup and event handling logic
3. Implement data binding to model

### Step 5: Implement Controller
1. Create `SSHAgentPageController` as coordinator
2. Wire together model, view, and service components
3. Implement EntryPageController interface

### Step 6: Integration
1. Modify EditEntryWidget to use page controller
2. Update build system and dependencies
3. Add comprehensive tests

## Code Migration Examples

### Before (Current Implementation)
```cpp
// In EditEntryWidget::updateSSHAgentKeyInfo()
void EditEntryWidget::updateSSHAgentKeyInfo()
{
    m_sshAgentUi->addToAgentButton->setEnabled(false);
    m_sshAgentUi->removeFromAgentButton->setEnabled(false);
    m_sshAgentUi->copyToClipboardButton->setEnabled(false);
    m_sshAgentUi->fingerprintTextLabel->setText(tr("n/a"));
    m_sshAgentUi->commentTextLabel->setText(tr("n/a"));
    m_sshAgentUi->decryptButton->setEnabled(false);
    m_sshAgentUi->publicKeyEdit->document()->setPlainText("");

    OpenSSHKey key;
    if (!getOpenSSHKey(key)) {
        return;
    }

    if (!key.fingerprint().isEmpty()) {
        m_sshAgentUi->fingerprintTextLabel->setText(
            key.fingerprint(QCryptographicHash::Md5) + "\n" +
            key.fingerprint(QCryptographicHash::Sha256));
    }
    // ... more UI manipulation mixed with business logic
}
```

### After (Proposed Implementation)
```cpp
// In SSHKeyDataModel::updateKeyInfo()
void SSHKeyDataModel::updateKeyInfo()
{
    if (!m_key || !m_valid) {
        clearKeyData();
        emit validationStateChanged(false);
        return;
    }

    emit validationStateChanged(true);
    emit keyDataChanged();
}

// In SSHAgentEditWidget::onKeyDataChanged() 
void SSHAgentEditWidget::onKeyDataChanged()
{
    bool valid = m_keyModel->isValid();
    
    m_ui->addToAgentButton->setEnabled(valid);
    m_ui->removeFromAgentButton->setEnabled(valid);
    m_ui->copyToClipboardButton->setEnabled(valid);
    m_ui->decryptButton->setEnabled(valid && m_keyModel->isEncrypted());
    
    if (valid) {
        m_ui->fingerprintTextLabel->setText(
            m_keyModel->fingerprint(QCryptographicHash::Md5) + "\n" +
            m_keyModel->fingerprint(QCryptographicHash::Sha256));
        m_ui->commentTextLabel->setText(m_keyModel->comment());
        m_ui->publicKeyEdit->setPlainText(m_keyModel->publicKey());
    } else {
        m_ui->fingerprintTextLabel->setText(tr("n/a"));
        m_ui->commentTextLabel->setText(tr("n/a"));
        m_ui->publicKeyEdit->clear();
    }
}
```

## Testing Strategy

### Unit Tests
```cpp
// Test key data model independently
class TestSSHKeyDataModel : public QObject
{
    Q_OBJECT

private slots:
    void testLoadFromAttachment();
    void testLoadFromFile();
    void testKeyValidation();
    void testDecryption();
    void testFingerprinting();
};

// Test agent service independently  
class TestSSHAgentService : public QObject
{
    Q_OBJECT

private slots:
    void testAgentConnection();
    void testKeyAddition();
    void testKeyRemoval();
    void testConstraints();
};
```

### Integration Tests
```cpp
// Test full SSH Agent page functionality
class TestSSHAgentController : public QObject
{
    Q_OBJECT

private slots:
    void testEntryLoading();
    void testEntrySaving();
    void testUserWorkflows();
    void testErrorHandling();
};
```

## Benefits of This Approach

1. **Separation of Concerns**: Each component has a single, well-defined responsibility
2. **Testability**: Components can be tested independently with mock dependencies
3. **Maintainability**: Changes to one component don't affect others
4. **Reusability**: Components can be reused in other contexts
5. **Extensibility**: New features can be added without modifying existing code

## Risk Mitigation

1. **Incremental Migration**: Implement new components alongside existing code
2. **Feature Flags**: Use configuration to switch between old and new implementations
3. **Comprehensive Testing**: Ensure new implementation matches existing behavior
4. **Backward Compatibility**: Maintain existing APIs during transition period

This refactoring approach provides a clear path toward better architecture while minimizing risk and maintaining functionality.