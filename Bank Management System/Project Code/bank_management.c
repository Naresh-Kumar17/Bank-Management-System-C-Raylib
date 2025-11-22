#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "raylib.h"

// Struct for account
typedef struct {
    char name[50];
    char father_name[50];
    char mobile_number[12];
    char address[100];
    char password[20];
    int account_number;
    float balance;
} Account;

// Struct for TextBox
typedef struct {
    Rectangle rect;
    char text[100];
    int active;
    int maxLength;
    int numericType; // 0 = any, 1 = digits only, 2 = number with decimal point
} TextBox;

// Struct for Button
typedef struct {
    Rectangle rect;
    const char *text;
    Color color;
} Button;

// Enum for states
typedef enum {
    MAIN_MENU,
    CREATE_ACCOUNT,
    LOGIN,
    USER_MENU,
    CHECK_BALANCE,
    UPDATE_INFO,
    DEPOSIT,
    DEPOSIT_SUCCESS,
    WITHDRAW,
    WITHDRAW_VERIFY,
    WITHDRAW_SUCCESS,
    WITHDRAW_FAILED,
    VIEW_HISTORY,
    LOGOUT,
    CONFIRM_DELETE
} State;

// File name
const char *ACCOUNT_FILE = "accounts.txt";  

int generateAccountNumber() {
    static int counter = 25-2500;
    
    // Try to find the highest account number in file
    FILE *file = fopen(ACCOUNT_FILE, "r");
    if (file) {
        char line[300];
        int maxNum =25-2500;
        Account acc;
        while (fgets(line, sizeof(line), file)) {
            if (sscanf(line, "%49[^,],%49[^,],%11[^,],%99[^,],%19[^,],%d,%f", acc.name, acc.father_name, acc.mobile_number, acc.address, acc.password, &acc.account_number, &acc.balance) == 7) {
                if (acc.account_number >= maxNum) {
                    maxNum = acc.account_number + 1;
                }
            }
        }
        fclose(file);
        counter = maxNum;
    }
    
    return counter++;
}

// Helper to get current date and time in PKT (Pakistan International Time, UTC+5)
void getCurrentDateTime(char *datetime) {
    time_t t = time(NULL);
    struct tm tm = *gmtime(&t);  // Get UTC time
    // Add 5 hours for PKT
    tm.tm_hour += 5;
    if (tm.tm_hour >= 24) {
        tm.tm_hour -= 24;
        tm.tm_mday += 1;
        // Note: This simple adjustment doesn't handle month/year rollover, but for basic use it's fine
    }
    sprintf(datetime, "%02d/%02d/%04d %02d:%02d:%02d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

// Helper to log transaction
void logTransaction(int account_number, const char *type, float amount, float new_balance) {
    char filename[50];
    sprintf(filename, "transactions_%d.txt", account_number);
    FILE *file = fopen(filename, "a");
    if (file) {
        char datetime[20];
        getCurrentDateTime(datetime);
        fprintf(file, "%s: %s %.2f, Balance: %.2f\n", datetime, type, amount, new_balance);
        fclose(file);
    }
}

// Global variables - declared early for use in functions
int buttonPressed = 0;  // Debounce flag for buttons

// Helper function to draw rounded rectangle
void DrawRoundedRectangle(Rectangle rec, float roundness, int segments, Color color) {
    DrawRectangleRounded(rec, roundness, segments, color);
}

// Helper function to draw rounded button without border
void DrawRoundedButton(Button *btn, float roundness, int segments) {
    DrawRoundedRectangle(btn->rect, roundness, segments, btn->color);
    // No border - clean modern look
    int textWidth = MeasureText(btn->text, 20);
    DrawText(btn->text, btn->rect.x + (btn->rect.width - textWidth) / 2, btn->rect.y + (btn->rect.height - 20) / 2, 20, WHITE);
}

// Draw TextBox
void DrawTextBox(TextBox *tb) {
    DrawRectangleRounded(tb->rect, 0.08f, 8, tb->active ? (Color){230, 230, 240, 255} : (Color){245, 245, 250, 255});
    DrawRectangleRoundedLines(tb->rect, 0.08f, 8, (Color){25, 55, 109, 255});
    // Vertically center the text inside the input
    int textY = (int)(tb->rect.y + (tb->rect.height - 20) / 2);
    DrawText(tb->text, tb->rect.x + 8, textY, 20, (Color){25, 55, 109, 255});
}

// Draw a label to the left of a TextBox, vertically centered
void DrawLabelLeft(TextBox *tb, const char *label) {
    int labelWidth = MeasureText(label, 20);
    int x = (int)tb->rect.x - labelWidth - 12; // 12px padding
    int y = (int)(tb->rect.y + (tb->rect.height - 20) / 2);
    DrawText(label, x, y, 20, BLACK);
}

// Handle TextBox input
void HandleTextBox(TextBox *tb) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), tb->rect)) {
        tb->active = 1;
    } else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        tb->active = 0;
    }

    if (tb->active) {
        int key = GetCharPressed();
        while (key > 0) {
            int len = strlen(tb->text);
            if (strlen(tb->text) < (size_t)tb->maxLength) {
                if (tb->numericType == 0) {
                    if ((key >= 32) && (key <= 125)) {
                        tb->text[len] = (char)key;
                        tb->text[len + 1] = '\0';
                    }
                } else if (tb->numericType == 1) { // digits only
                    if (key >= '0' && key <= '9') {
                        tb->text[len] = (char)key;
                        tb->text[len + 1] = '\0';
                    }
                } else if (tb->numericType == 2) { // number with optional single decimal point
                    if ((key >= '0' && key <= '9') || (key == '.')) {
                        // allow only one decimal point
                        if (key == '.') {
                            if (strchr(tb->text, '.') == NULL) {
                                tb->text[len] = (char)key;
                                tb->text[len + 1] = '\0';
                            }
                        } else {
                            tb->text[len] = (char)key;
                            tb->text[len + 1] = '\0';
                        }
                    }
                }
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) && strlen(tb->text) > 0) {
            tb->text[strlen(tb->text) - 1] = '\0';
        }
    }
}

// Draw Button
void DrawButton(Button *btn) {
    DrawRoundedButton(btn, 0.15f, 12);
}

// Check if button is clicked (with debounce)
int IsButtonClicked(Button *btn) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), btn->rect) && !buttonPressed) {
        buttonPressed = 1;
        return 1;
    }
    if (!IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        buttonPressed = 0;
    }
    return 0;
}

// Update information - allows updating user details
void updateInformation(Account *user) {
    FILE *file = fopen(ACCOUNT_FILE, "r");
    FILE *tempFile = fopen("temp.txt", "w");
    char line[300];
    while (fgets(line, sizeof(line), file)) {
        Account acc;
        if (sscanf(line, "%49[^,],%49[^,],%11[^,],%99[^,],%19[^,],%d,%f", acc.name, acc.father_name, acc.mobile_number, acc.address, acc.password, &acc.account_number, &acc.balance) == 7) {
            if (acc.account_number == user->account_number) {
                fprintf(tempFile, "%s,%s,%s,%s,%s,%d,%.2f\n", user->name, user->father_name, user->mobile_number, user->address, user->password, user->account_number, user->balance);
            } else {
                fprintf(tempFile, "%s", line);
            }
        }
    }
    fclose(file);
    fclose(tempFile);
    remove(ACCOUNT_FILE);
    rename("temp.txt", ACCOUNT_FILE);
}

// Delete account - removes user account
void deleteAccount(Account *user) {
    FILE *file = fopen(ACCOUNT_FILE, "r");
    FILE *tempFile = fopen("temp.txt", "w");
    char line[300];
    while (fgets(line, sizeof(line), file)) {
        Account acc;
        if (sscanf(line, "%49[^,],%49[^,],%11[^,],%99[^,],%19[^,],%d,%f", acc.name, acc.father_name, acc.mobile_number, acc.address, acc.password, &acc.account_number, &acc.balance) == 7) {
            if (acc.account_number != user->account_number) {
                fprintf(tempFile, "%s", line);
            }
        }
    }
    fclose(file);
    fclose(tempFile);
    remove(ACCOUNT_FILE);
    rename("temp.txt", ACCOUNT_FILE);

    char filename[50];
    sprintf(filename, "transactions_%d.txt", user->account_number);
    remove(filename);
}

// Deposit money - adds money to balance
void depositMoney(Account *user) {
    FILE *file = fopen(ACCOUNT_FILE, "r");
    FILE *tempFile = fopen("temp.txt", "w");
    char line[300];
    while (fgets(line, sizeof(line), file)) {
        Account acc;
        if (sscanf(line, "%49[^,],%49[^,],%11[^,],%99[^,],%19[^,],%d,%f", acc.name, acc.father_name, acc.mobile_number, acc.address, acc.password, &acc.account_number, &acc.balance) == 7) {
            if (acc.account_number == user->account_number) {
                fprintf(tempFile, "%s,%s,%s,%s,%s,%d,%.2f\n", acc.name, acc.father_name, acc.mobile_number, acc.address, acc.password, acc.account_number, user->balance);
            } else {
                fprintf(tempFile, "%s", line);
            }
        }
    }
    fclose(file);
    fclose(tempFile);
    remove(ACCOUNT_FILE);
    rename("temp.txt", ACCOUNT_FILE);
}

// Withdraw money - subtracts money from balance
void withdrawMoney(Account *user) {
    FILE *file = fopen(ACCOUNT_FILE, "r");
    FILE *tempFile = fopen("temp.txt", "w");
    char line[300];
    while (fgets(line, sizeof(line), file)) {
        Account acc;
        if (sscanf(line, "%49[^,],%49[^,],%11[^,],%99[^,],%19[^,],%d,%f", acc.name, acc.father_name, acc.mobile_number, acc.address, acc.password, &acc.account_number, &acc.balance) == 7) {
            if (acc.account_number == user->account_number) {
                fprintf(tempFile, "%s,%s,%s,%s,%s,%d,%.2f\n", acc.name, acc.father_name, acc.mobile_number, acc.address, acc.password, acc.account_number, user->balance);
            } else {
                fprintf(tempFile, "%s", line);
            }
        }
    }
    fclose(file);
    fclose(tempFile);
    remove(ACCOUNT_FILE);
    rename("temp.txt", ACCOUNT_FILE);
}

// Global variables
State currentState = MAIN_MENU;
Account currentUser;
char message[200] = "";
int messageTimer = 0;
int accountCreatedSuccessfully = 0;  // Flag to show login button after account creation
float pendingWithdrawAmount = 0.0f;
int withdrawQuestionIndex = -1;
float depositSuccessAmount = 0.0f;
int depositSuccessTimer = 0;
float withdrawSuccessAmount = 0.0f;
int withdrawSuccessTimer = 0;
int withdrawFailedTimer = 0;
int logoutTimer = 0;

// Main function
int main() {
    InitWindow(800, 600, "Bank Management System");
    SetTargetFPS(60);
    srand((unsigned)time(NULL));

    // Define UI elements with professional banking colors
    Button btnCreate = {{250, 180, 300, 60}, "Create Account", (Color){25, 55, 109, 255}};
    Button btnLogin = {{250, 280, 300, 60}, "Login", (Color){25, 55, 109, 255}};
    Button btnExit = {{250, 380, 300, 60}, "Exit", (Color){191, 144, 0, 255}};

    Button btnSubmitCreate = {{300, 500, 240, 56}, "Submit", (Color){25, 55, 109, 255}};
    TextBox tbName = {{200, 110, 400, 44}, "", 0, 49, 0};
    TextBox tbFatherName = {{200, 174, 400, 44}, "", 0, 49, 0};
    TextBox tbMobile = {{200, 238, 400, 44}, "", 0, 11, 1};
    TextBox tbAddress = {{200, 302, 400, 44}, "", 0, 99, 0};
    TextBox tbPassword = {{200, 366, 400, 44}, "", 0, 19, 0};

    Button btnSubmitLogin = {{300, 360, 240, 56}, "Login", (Color){25, 55, 109, 255}};
    TextBox tbLoginMobile = {{200, 170, 400, 44}, "", 0, 11, 1};
    TextBox tbLoginPassword = {{200, 236, 400, 44}, "", 0, 19, 0};
    
    // Confirm delete UI elements
    Button btnConfirmDelete = {{260, 300, 280, 56}, "Confirm Delete", (Color){200, 50, 50, 255}};
    Button btnCancelDelete = {{560, 300, 100, 56}, "Cancel", (Color){100,100,100,255}};
    TextBox tbConfirmPassword = {{260, 240, 400, 44}, "", 0, 19, 0};

    Button btnCheckBalance = {{200, 100, 400, 50}, "Check Balance", (Color){25, 55, 109, 255}};
    Button btnUpdateInfo = {{200, 170, 400, 50}, "Update Info", (Color){191, 144, 0, 255}};
    Button btnDeposit = {{200, 240, 400, 50}, "Deposit", (Color){25, 55, 109, 255}};
    Button btnWithdraw = {{200, 310, 400, 50}, "Withdraw", (Color){25, 55, 109, 255}};
    Button btnViewHistory = {{200, 450, 400, 50}, "View History", (Color){191, 144, 0, 255}};
    Button btnDelete = {{200, 520, 400, 50}, "Delete Account", (Color){200, 50, 50, 255}};
    Button btnLogout = {{200, 380, 400, 50}, "Logout", (Color){100, 100, 100, 255}};

    Button btnSubmitUpdate = {{300, 420, 200, 50}, "Update", (Color){191, 144, 0, 255}};
    TextBox tbUpdateName = {{250, 100, 300, 30}, "", 0, 49};
    TextBox tbUpdateFather = {{250, 150, 300, 30}, "", 0, 49};
    TextBox tbUpdateAddress = {{250, 200, 300, 30}, "", 0, 99};
    TextBox tbUpdatePassword = {{250, 250, 300, 30}, "", 0, 19};

    Button btnSubmitDeposit = {{300, 230, 200, 50}, "Deposit", (Color){25, 55, 109, 255}};
    TextBox tbDepositAmount = {{250, 150, 300, 30}, "", 0, 10, 2};

    Button btnSubmitWithdraw = {{300, 230, 200, 50}, "Withdraw", (Color){25, 55, 109, 255}};
    TextBox tbWithdrawAmount = {{250, 150, 300, 30}, "", 0, 10, 2};

    Button btnVerifyWithdraw = {{260, 230, 140, 50}, "Verify", (Color){25, 55, 109, 255}};
    Button btnCancelVerify = {{420, 230, 140, 50}, "Cancel", (Color){100,100,100,255}};
    TextBox tbWithdrawSecurity = {{260, 180, 300, 44}, "", 0, 50, 0};

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground((Color){255, 255, 255, 255});

        Button btnBack;
        switch (currentState) {
            case MAIN_MENU:
                DrawText("Bank Management System", 250, 100, 30, BLACK);
                DrawButton(&btnCreate);
                DrawButton(&btnLogin);
                DrawButton(&btnExit);

                if (IsButtonClicked(&btnCreate)) {
                    currentState = CREATE_ACCOUNT;
                    strcpy(message, "");
                } else if (IsButtonClicked(&btnLogin)) {
                    currentState = LOGIN;
                    strcpy(message, "");
                } else if (IsButtonClicked(&btnExit)) {
                    CloseWindow();
                    return 0;
                }
                break;

            case CREATE_ACCOUNT:
                DrawText("Create New Account", 300, 50, 25, BLACK);
                
                if (!accountCreatedSuccessfully) {
                    // Show form for creating account
                    // Draw labels above the input fields to avoid overlap
                    DrawLabelLeft(&tbName, "Name:");
                    DrawTextBox(&tbName);
                    DrawLabelLeft(&tbFatherName, "Father's Name:");
                    DrawTextBox(&tbFatherName);
                    DrawLabelLeft(&tbMobile, "Mobile (11 digits):");
                    DrawTextBox(&tbMobile);
                    DrawLabelLeft(&tbAddress, "Address:");
                    DrawTextBox(&tbAddress);
                    DrawLabelLeft(&tbPassword, "Password:");
                    DrawTextBox(&tbPassword);
                    DrawButton(&btnSubmitCreate);

                    HandleTextBox(&tbName);
                    HandleTextBox(&tbFatherName);
                    HandleTextBox(&tbMobile);
                    HandleTextBox(&tbAddress);
                    HandleTextBox(&tbPassword);

                    if (IsButtonClicked(&btnSubmitCreate)) {
                        if (strlen(tbName.text) == 0 || strlen(tbFatherName.text) == 0 || strlen(tbMobile.text) != 11 || strlen(tbAddress.text) == 0 || strlen(tbPassword.text) == 0) {
                            strcpy(message, "All fields must be filled correctly!");
                            messageTimer = 180;  // 3 seconds at 60 FPS
                        } else {
                            // Check if mobile number already exists
                            int mobileExists = 0;
                            FILE *fileCheck = fopen(ACCOUNT_FILE, "r");
                            if (fileCheck) {
                                char lineChk[300];
                                Account tempAcc;
                                while (fgets(lineChk, sizeof(lineChk), fileCheck)) {
                                    if (sscanf(lineChk, "%49[^,],%49[^,],%11[^,],%99[^,],%19[^,],%d,%f", tempAcc.name, tempAcc.father_name, tempAcc.mobile_number, tempAcc.address, tempAcc.password, &tempAcc.account_number, &tempAcc.balance) == 7) {
                                        if (strcmp(tempAcc.mobile_number, tbMobile.text) == 0) {
                                            mobileExists = 1;
                                            break;
                                        }
                                    }
                                }
                                fclose(fileCheck);
                            }

                            if (mobileExists) {
                                strcpy(message, "Mobile number already exists!");
                                messageTimer = 180;
                            } else {
                                Account acc;
                                strcpy(acc.name, tbName.text);
                                strcpy(acc.father_name, tbFatherName.text);
                                strcpy(acc.mobile_number, tbMobile.text);
                                strcpy(acc.address, tbAddress.text);
                                strcpy(acc.password, tbPassword.text);

                                acc.account_number = generateAccountNumber();
                                acc.balance = 0.0;

                                FILE *file = fopen(ACCOUNT_FILE, "a");
                                if (file) {
                                    fprintf(file, "%s,%s,%s,%s,%s,%d,%.2f\n", acc.name, acc.father_name, acc.mobile_number, acc.address, acc.password, acc.account_number, acc.balance);
                                    fclose(file);
                                    sprintf(message, "Account created! Number: %d", acc.account_number);
                                    messageTimer = 180;
                                    // Clear text boxes
                                    strcpy(tbName.text, "");
                                    strcpy(tbFatherName.text, "");
                                    strcpy(tbMobile.text, "");
                                    strcpy(tbAddress.text, "");
                                    strcpy(tbPassword.text, "");
                                    accountCreatedSuccessfully = 1;  // Set flag to show login button
                                } else {
                                    strcpy(message, "Unable to save account!");
                                    messageTimer = 180;
                                }
                            }
                        }
                    }
                } else {
                    // Show message and login button after account created
                    DrawText(message, 150, 180, 20, (Color){25, 55, 109, 255});
                    DrawText("Click Login to continue", 120, 250, 18, BLACK);
                    Button btnGoToLogin = {{250, 330, 300, 60}, "Go to Login", (Color){25, 55, 109, 255}};
                    DrawButton(&btnGoToLogin);
                    
                    if (IsButtonClicked(&btnGoToLogin)) {
                        currentState = LOGIN;
                        accountCreatedSuccessfully = 0;  // Reset flag
                        strcpy(message, "");
                        strcpy(tbLoginMobile.text, "");
                        strcpy(tbLoginPassword.text, "");
                    }
                }
                break;

            case LOGIN:
                DrawText("Login", 350, 100, 25, BLACK);
                // Draw labels to the left of the input fields (same-line)
                DrawLabelLeft(&tbLoginMobile, "Mobile:");
                DrawTextBox(&tbLoginMobile);
                DrawLabelLeft(&tbLoginPassword, "Password:");
                DrawTextBox(&tbLoginPassword);
                DrawButton(&btnSubmitLogin);

                HandleTextBox(&tbLoginMobile);
                HandleTextBox(&tbLoginPassword);

                if (IsButtonClicked(&btnSubmitLogin)) {
                    FILE *file = fopen(ACCOUNT_FILE, "r");
                    if (!file) {
                        strcpy(message, "No accounts found!");
                        messageTimer = 180;
                    } else {
                        char line[300];
                        int found = 0;
                        while (fgets(line, sizeof(line), file)) {
                            if (sscanf(line, "%49[^,],%49[^,],%11[^,],%99[^,],%19[^,],%d,%f", currentUser.name, currentUser.father_name, currentUser.mobile_number, currentUser.address, currentUser.password, &currentUser.account_number, &currentUser.balance) == 7) {
                                if (strcmp(currentUser.mobile_number, tbLoginMobile.text) == 0 && strcmp(currentUser.password, tbLoginPassword.text) == 0) {
                                    found = 1;
                                    break;
                                }
                            }
                        }
                        fclose(file);
                        if (found) {
                            currentState = USER_MENU;
                            strcpy(message, "");
                            strcpy(tbLoginMobile.text, "");
                            strcpy(tbLoginPassword.text, "");
                        } else {
                            strcpy(message, "Invalid credentials!");
                            messageTimer = 180;
                        }
                    }
                }
                break;

            case USER_MENU:
                DrawText("User Menu", 330, 50, 25, BLACK);
                DrawButton(&btnCheckBalance);
                DrawButton(&btnUpdateInfo);
                DrawButton(&btnDelete);
                DrawButton(&btnDeposit);
                DrawButton(&btnWithdraw);
                DrawButton(&btnViewHistory);
                DrawButton(&btnLogout);

                if (IsButtonClicked(&btnCheckBalance)) {
                    currentState = CHECK_BALANCE;
                } else if (IsButtonClicked(&btnUpdateInfo)) {
                    currentState = UPDATE_INFO;
                    strcpy(tbUpdateName.text, currentUser.name);
                    strcpy(tbUpdateFather.text, currentUser.father_name);
                    strcpy(tbUpdateAddress.text, currentUser.address);
                    strcpy(tbUpdatePassword.text, currentUser.password);
                } else if (IsButtonClicked(&btnDelete)) {
                    // Go to confirmation screen before deleting
                    currentState = CONFIRM_DELETE;
                    strcpy(tbConfirmPassword.text, "");
                } else if (IsButtonClicked(&btnDeposit)) {
                    currentState = DEPOSIT;
                } else if (IsButtonClicked(&btnWithdraw)) {
                    currentState = WITHDRAW;
                } else if (IsButtonClicked(&btnViewHistory)) {
                    currentState = VIEW_HISTORY;
                } else if (IsButtonClicked(&btnLogout)) {
                    logoutTimer = 240;  // 4 seconds at 60 FPS
                    currentState = LOGOUT;
                }
                break;

            case CHECK_BALANCE:
                DrawText("Check Balance", 300, 100, 25, BLACK);
                char balanceStr[50];
                sprintf(balanceStr, "Balance: %.2f", currentUser.balance);
                DrawText(balanceStr, 300, 200, 20, BLACK);
                btnBack.rect.x = 350; btnBack.rect.y = 300; btnBack.rect.width = 100; btnBack.rect.height = 40;
                btnBack.text = "Back"; btnBack.color = GRAY;
                DrawButton(&btnBack);
                if (IsButtonClicked(&btnBack)) {
                    currentState = USER_MENU;
                }
                break;

            case UPDATE_INFO:
                DrawText("Update Information", 280, 50, 25, BLACK);
                DrawLabelLeft(&tbUpdateName, "Name:");
                DrawTextBox(&tbUpdateName);
                DrawLabelLeft(&tbUpdateFather, "Father's Name:");
                DrawTextBox(&tbUpdateFather);
                DrawLabelLeft(&tbUpdateAddress, "Address:");
                DrawTextBox(&tbUpdateAddress);
                DrawLabelLeft(&tbUpdatePassword, "Password:");
                DrawTextBox(&tbUpdatePassword);
                DrawButton(&btnSubmitUpdate);
                HandleTextBox(&tbUpdateName);
                HandleTextBox(&tbUpdateFather);
                HandleTextBox(&tbUpdateAddress);
                HandleTextBox(&tbUpdatePassword);
                if (IsButtonClicked(&btnSubmitUpdate)) {
                    strcpy(currentUser.name, tbUpdateName.text);
                    strcpy(currentUser.father_name, tbUpdateFather.text);
                    strcpy(currentUser.address, tbUpdateAddress.text);
                    strcpy(currentUser.password, tbUpdatePassword.text);
                    updateInformation(&currentUser);
                    currentState = USER_MENU;
                }
                break;
            case DEPOSIT:
                DrawText("Deposit Money", 300, 50, 25, BLACK);
                DrawLabelLeft(&tbDepositAmount, "Amount:");
                DrawTextBox(&tbDepositAmount);
                DrawButton(&btnSubmitDeposit);
                btnBack.rect.x = 350; btnBack.rect.y = 310; btnBack.rect.width = 100; btnBack.rect.height = 40;
                btnBack.text = "Back"; btnBack.color = GRAY;
                DrawButton(&btnBack);
                HandleTextBox(&tbDepositAmount);
                if (IsButtonClicked(&btnSubmitDeposit)) {
                    float amount = atof(tbDepositAmount.text);
                    if (amount > 0) {
                        currentUser.balance += amount;
                        depositMoney(&currentUser);
                        logTransaction(currentUser.account_number, "Deposit", amount, currentUser.balance);
                        depositSuccessAmount = amount;
                        depositSuccessTimer = 240;  // 4 seconds at 60 FPS
                        strcpy(tbDepositAmount.text, "");
                        currentState = DEPOSIT_SUCCESS;
                    } else {
                        strcpy(message, "Enter a valid amount!");
                        messageTimer = 180;
                    }
                }
                if (IsButtonClicked(&btnBack)) {
                    currentState = USER_MENU;
                    strcpy(tbDepositAmount.text, "");
                }
                break;
            case DEPOSIT_SUCCESS:
                DrawText("Deposit Successful", 280, 100, 30, BLACK);
                char depositMsg[100];
                sprintf(depositMsg, "Amount %.2f submitted successfully!", depositSuccessAmount);
                DrawText(depositMsg, 180, 200, 20, (Color){0, 128, 0, 255});
                DrawText("Returning to menu...", 250, 300, 18, GRAY);
                depositSuccessTimer--;
                if (depositSuccessTimer <= 0) {
                    currentState = USER_MENU;
                    depositSuccessAmount = 0.0f;
                }
                break;
            case WITHDRAW:
                DrawText("Withdraw Money", 300, 50, 25, BLACK);
                if (currentUser.balance <= 0.0f) {
                    DrawText("No funds available in this account.", 220, 200, 20, RED);
                    btnBack.rect.x = 350; btnBack.rect.y = 300; btnBack.rect.width = 100; btnBack.rect.height = 40;
                    btnBack.text = "Back"; btnBack.color = GRAY;
                    DrawButton(&btnBack);
                    if (IsButtonClicked(&btnBack)) {
                        currentState = USER_MENU;
                    }
                } else {
                    DrawLabelLeft(&tbWithdrawAmount, "Amount:");
                    DrawTextBox(&tbWithdrawAmount);
                    DrawButton(&btnSubmitWithdraw);
                    btnBack.rect.x = 350; btnBack.rect.y = 310; btnBack.rect.width = 100; btnBack.rect.height = 40;
                    btnBack.text = "Back"; btnBack.color = GRAY;
                    DrawButton(&btnBack);
                    HandleTextBox(&tbWithdrawAmount);
                    if (IsButtonClicked(&btnSubmitWithdraw)) {
                        float amount = atof(tbWithdrawAmount.text);
                        if (amount > 0 && amount <= currentUser.balance) {
                            if (amount > 50000.0f) {
                                // Require security question verification for withdrawals > 5000
                                pendingWithdrawAmount = amount;
                                withdrawQuestionIndex = rand() % 4;
                                strcpy(tbWithdrawSecurity.text, "");
                                currentState = WITHDRAW_VERIFY;
                            } else {
                                currentUser.balance -= amount;
                                withdrawMoney(&currentUser);
                                logTransaction(currentUser.account_number, "Withdraw", amount, currentUser.balance);
                                withdrawSuccessAmount = amount;
                                withdrawSuccessTimer = 240;  // 4 seconds at 60 FPS
                                strcpy(tbWithdrawAmount.text, "");
                                currentState = WITHDRAW_SUCCESS;
                            }
                        } else {
                            strcpy(message, "Insufficient amount! please enter a valid amount.");
                            messageTimer = 180;
                        }
                    }
                    if (IsButtonClicked(&btnBack)) {
                        currentState = USER_MENU;
                        strcpy(tbWithdrawAmount.text, "");
                    }
                }
                break;
            case WITHDRAW_VERIFY:
                DrawText("Verify Withdrawal", 300, 50, 25, BLACK);
                {
                    char qbuf[200];
                    char expected[100];
                    switch (withdrawQuestionIndex) {
                        case 0:
                            sprintf(qbuf, "What is your father's name?");
                            strcpy(expected, currentUser.father_name);
                            break;
                        case 1:
                            sprintf(qbuf, "What is your registered mobile number?");
                            strcpy(expected, currentUser.mobile_number);
                            break;
                        case 2:
                            sprintf(qbuf, "What is your address?");
                            strcpy(expected, currentUser.address);
                            break;
                        case 3:
                            sprintf(qbuf, "What is your account number?");
                            sprintf(expected, "%d", currentUser.account_number);
                            break;
                        default:
                            sprintf(qbuf, "Security question:");
                            expected[0] = '\0';
                            break;
                    }
                    DrawText(qbuf, 100, 120, 20, BLACK);
                    DrawLabelLeft(&tbWithdrawSecurity, "Answer:");
                    DrawTextBox(&tbWithdrawSecurity);
                    DrawButton(&btnVerifyWithdraw);
                    DrawButton(&btnCancelVerify);
                    HandleTextBox(&tbWithdrawSecurity);

                    if (IsButtonClicked(&btnVerifyWithdraw)) {
                        if (strcmp(tbWithdrawSecurity.text, expected) == 0) {
                            float amount = pendingWithdrawAmount;
                            currentUser.balance -= amount;
                            withdrawMoney(&currentUser);
                            logTransaction(currentUser.account_number, "Withdraw", amount, currentUser.balance);
                            strcpy(tbWithdrawAmount.text, "");
                            withdrawSuccessAmount = amount;
                            withdrawSuccessTimer = 240;  // 4 seconds at 60 FPS
                            pendingWithdrawAmount = 0.0f;
                            withdrawQuestionIndex = -1;
                            strcpy(tbWithdrawSecurity.text, "");
                            currentState = WITHDRAW_SUCCESS;
                        } else {
                            withdrawFailedTimer = 240;  // 4 seconds at 60 FPS
                            currentState = WITHDRAW_FAILED;
                            strcpy(tbWithdrawSecurity.text, "");
                        }
                    }
                    if (IsButtonClicked(&btnCancelVerify)) {
                        currentState = USER_MENU;
                    }
                }
                break;
            case WITHDRAW_FAILED:
                DrawText("Withdrawal Failed", 300, 100, 30, BLACK);
                DrawText("Incorrect answer!", 280, 200, 24, RED);
                DrawText("Withdrawal cancelled.", 250, 260, 20, BLACK);
                DrawText("Returning to menu...", 250, 340, 18, GRAY);
                withdrawFailedTimer--;
                if (withdrawFailedTimer <= 0) {
                    currentState = USER_MENU;
                    pendingWithdrawAmount = 0.0f;
                    withdrawQuestionIndex = -1;
                }
                break;
            case WITHDRAW_SUCCESS:
                DrawText("Withdrawal Successful", 270, 100, 30, BLACK);
                char withdrawMsg[100];
                sprintf(withdrawMsg, "Amount %.2f withdrawn successfully!", withdrawSuccessAmount);
                DrawText(withdrawMsg, 160, 200, 20, (Color){0, 128, 0, 255});
                DrawText("Returning to menu...", 250, 300, 18, GRAY);
                withdrawSuccessTimer--;
                if (withdrawSuccessTimer <= 0) {
                    currentState = USER_MENU;
                    withdrawSuccessAmount = 0.0f;
                }
                break;
            case VIEW_HISTORY:
                DrawText("Transaction History", 280, 50, 25, BLACK);
                {
                    char filename[50];
                    sprintf(filename, "transactions_%d.txt", currentUser.account_number);
                    FILE *file = fopen(filename, "r");
                    if (file) {
                            char line[200];
                            int y = 100;
                            // Increased font size for better readability
                            while (fgets(line, sizeof(line), file) && y < 550) {
                                DrawText(line, 50, y, 20, BLACK);
                                y += 28;
                            }
                            fclose(file);
                        } else {
                            DrawText("No transactions found.", 300, 200, 20, BLACK);
                        }
                }
                btnBack.rect.x = 350; btnBack.rect.y = 500; btnBack.rect.width = 100; btnBack.rect.height = 40;
                btnBack.text = "Back"; btnBack.color = GRAY;
                DrawButton(&btnBack);

                if (IsButtonClicked(&btnBack)) {
                    currentState = USER_MENU;
                }
                break;
            case LOGOUT:
                DrawText("Thank You!", 320, 150, 40, (Color){25, 55, 109, 255});
                DrawText("Thanks for visiting our bank", 210, 250, 24, BLACK);
                DrawText("Returning to main menu...", 240, 340, 18, GRAY);
                logoutTimer--;
                if (logoutTimer <= 0) {
                    currentState = MAIN_MENU;
                }
                break;
            case CONFIRM_DELETE:
                DrawText("Confirm Delete", 300, 180, 25, BLACK);
                DrawLabelLeft(&tbConfirmPassword, "Password:");
                DrawTextBox(&tbConfirmPassword);
                DrawButton(&btnConfirmDelete);
                DrawButton(&btnCancelDelete);
                HandleTextBox(&tbConfirmPassword);
                if (IsButtonClicked(&btnConfirmDelete)) {
                    if (strcmp(tbConfirmPassword.text, currentUser.password) == 0) {
                        deleteAccount(&currentUser);
                        strcpy(message, "Account deleted.");
                        messageTimer = 180;
                        currentState = MAIN_MENU;
                    } else {
                        strcpy(message, "Incorrect password!");
                        messageTimer = 180;
                    }
                }
                if (IsButtonClicked(&btnCancelDelete)) {
                    currentState = USER_MENU;
                }
                break;
        }
        // Display message if any
        if (messageTimer > 0) {
            DrawText(message, 200, 550, 20, RED);
            messageTimer--;
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}