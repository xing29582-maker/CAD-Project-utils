#pragma once

#include "PlatformExport.h"
#include "NameDefine.h"

#include <memory>
#include <vector>

namespace cadutils
{
    class Document;
    class Transaction;

    class CADUTILS_PLATFORM_API TransactionManager
    {
    public:
        explicit TransactionManager(std::weak_ptr<Document> doc);
        ~TransactionManager() = default;

        // Begin a new transaction. Returns false if one is already active.
        bool BeginTransaction();

        // Commit the active transaction to the undo stack.
        // Clears the redo stack. Returns false if no active transaction.
        bool Commit();

        // Roll back the active transaction (discard changes, restore old values).
        bool RollBack();

        // Undo the top of the undo stack.
        bool Undo();

        // Redo the top of the redo stack.
        bool Redo();

        bool CanUndo() const noexcept { return !m_undoStack.empty(); }
        bool CanRedo() const noexcept { return !m_redoStack.empty(); }
        bool HasActiveTransaction() const noexcept { return m_active != nullptr; }

    private:
        // Apply changes from a transaction (used by Undo/Redo)
        // If useOldValues is true, applies oldValue (undo); otherwise applies newValue (redo).
        void ApplyTransaction(const Transaction& tx, bool useOldValues);

        std::weak_ptr<Document> m_doc;
        std::shared_ptr<Transaction> m_active;
        std::vector<std::shared_ptr<Transaction>> m_undoStack;
        std::vector<std::shared_ptr<Transaction>> m_redoStack;
    };
}