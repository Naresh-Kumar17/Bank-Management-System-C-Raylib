# 🏦 Bank Management System Using Raylib

> **University:** FAST NUCES Karachi  
> **Department:** Department of AI & Data Science  
> **Course:** Programming Fundamentals (Fall 2025)  
> **Authors:** Naresh Kumar (25K-2501) & Syed Zaid Khalid (25K-2522)  
> **Instructor:** Sir Jahanzaib

---

## 📖 Abstract

This **Bank Management System** is a GUI-based application developed in **C Language** and **Raylib for Gui**. It simulates a full-scale banking environment, allowing users to perform essential banking operations through a user-friendly menu interface. The system utilizes **File Handling** to ensure persistent data storage, meaning user records, balances, and transaction histories are saved even after the program is closed.



[Image of Bank Management System Use Case Diagram]


## 🚀 Key Features

This system mimics real-world banking functionalities, including:

* **🔐 Secure Login System:** Users must log in with a valid mobile number and password.
* **📝 Account Creation:** fast registration process capturing Name, Father's Name, Mobile, Address, and Password.
* **💰 Banking Operations:**
    * **Deposit Money:** Add funds to your account instantly.
    * **Withdraw Money:** Secure withdrawal checks. *Note: Withdrawals over 50,000 trigger a security question.*
    * **Check Balance:** View real-time account balance.
* **📜 Transaction History:** View a log of previous transactions.
* **✏️ Update Information:** Modify personal details (Name, Address, Password, etc.).
* **❌ Delete Account:** Permanently remove user records from the database.
* **💾 Persistent Data:** Uses file handling (`.txt` or binary files) to store login credentials and financial records permanently.

## 🛠️ Tech Stack

* **Language:** C
*  **Library For Gui :** Raylib
* **Concepts Used:** * File Handling (Read/Write/Append)
    * Pointers & Arrays
    * String Manipulation
    * Structures (`struct`)
    * Loops & Conditionals
* **Compatible Compilers:** GCC, Dev C++, Code::Blocks

---

## ⚙️ System Design & Flow

The system follows a linear flow control with a central loop that keeps the user logged in until they explicitly choose to exit or log out.



[Image of C program flowchart for banking system]


### Algorithm Overview
1.  **Start:** Display Login/Register Menu.
2.  **Authentication:** Verify credentials against the file database.
3.  **User Menu:** Present options (Deposit, Withdraw, History, etc.).
4.  **Processing:** Execute the selected function and update the specific file.
5.  **Loop:** Return to the menu for further actions.
6.  **Exit:** Save all states and close the application.

---

## 📂 Code Structure

The project relies on modular programming. Below are the primary functions driving the system:

| Function Name | Description |
| :--- | :--- |
| `createAccount()` | Captures user data and appends it to the login file. |
| `login()` | Authenticates user credentials against stored records. |
| `userMenu(*user)` | Displays the main dashboard after a successful login. |
| `checkBalance(*user)` | Reads and displays the current balance. |
| `depositMoney(*user)` | Adds amount to balance and updates the file. |
| `withdrawMoney(*user)` | Checks sufficient funds and deducts the amount. |
| `viewTransactionHistory(*user)` | Iterates through the transaction log to show history. |
| `deleteAccount(*user)` | Permanently removes the user's record from the file. |

---

## 🧪 Testing & Results

The system has undergone rigorous testing to ensure data integrity.

* **Login/Auth:** Verified valid vs. invalid credentials.
* **Persistence:** Confirmed data remains after closing and reopening the CLI.
* **Validation:** Ensures users cannot withdraw more money than their current balance.
* **Security Trigger:** Successfully triggers random security questions for high-value withdrawals (>50,000).


---

## 🎬 Project Demonstration

### Bank Management System Walkthrough

Watch the complete demonstration of the Bank Management System's features, including account creation, transaction logging, and data persistence.

<div align="center">
    <video src="[https://raw.githubusercontent.com/Naresh-Kumar17/Bank-Management-System-C-Raylib/main/Bank-Management-System-Video.mp4](https://github.com/Naresh-Kumar17/Bank-Management-System-C-Raylib/blob/main/Bank%20Management%20System/Project%20Quick%20Demonstration/Bank-Management-System-Demonstration.mp4)" 
           width="700" 
           autoplay 
           loop 
           muted 
           controls>
    </video>
</div>



---
## ⚠️ Limitations & Future Scope

### Current Limitations
* **Encryption:** Passwords are currently stored in plain text.
* **Recovery:** Deleted accounts are permanently removed immediately; no "Trash" or recovery period exists.
* **Concurrency:** Designed for a single user instance at a time.

### Future Enhancements
* [ ] Implement **2-Step Verification** for logins.
* [ ] Add **Encryption** for sensitive data storage.
* [ ] Create a **30-day recovery period** for deleted accounts.
* [ ] Add **Interest Calculation** for savings accounts.

------

*Submitted as the First Sesmter Project for Programming Fundamentals, Fall 2025.*
