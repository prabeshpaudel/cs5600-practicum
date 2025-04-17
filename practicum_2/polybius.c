/*
 * polybius.c / Assignment / Random Number Generation and Encryption
 *
 * Prabesh Paudel / CS5600 / Northeastern University
 * Spring 2024 / Jan 24, 2024
 *
 */

 #include <stdio.h> 
 #include <ctype.h> 
 #include <string.h> 
 
 #define TABLE_ROWS 6 // Number of rows in the table
 #define TABLE_COLS 6 // Number of columns in the table
 
 // Struct definition for the Polybius table
 typedef struct {
     int rows;                        // Number of rows
     int cols;                        // Number of columns
     char table[TABLE_ROWS][TABLE_COLS]; // The table contents
 } Table;
 
 // Initializes the Polybius table with predefined characters
 Table initializeMatrix() {
     Table t;
     t.rows = TABLE_ROWS;
     t.cols = TABLE_COLS;
 
     // Hardcoded table values for encryption
     char predefinedValues[TABLE_ROWS][TABLE_COLS] = {
         {'A', 'B', 'C', 'D', 'E', 'F'},
         {'G', 'H', 'I', 'J', 'K', 'L'},
         {'M', 'N', 'O', 'P', 'Q', 'R'},
         {'S', 'T', 'U', 'V', 'W', 'X'},
         {'Y', 'Z', '$', '.', '?', '!'},
         {',', ';', ':', '\'', '"', '-'}
     };
 
     // Copying the predefined values into the table struct
     for (int i = 0; i < t.rows; i++) {
         for (int j = 0; j < t.cols; j++) {
             t.table[i][j] = predefinedValues[i][j];
         }
     }
     return t; // Return the initialized table
 }
 
 // Finds the index (row and col as a two-digit number) for a given character
 int getIndices(Table *t, char letter) {
     for (int i = 0; i < t->rows; i++) {
         for (int j = 0; j < t->cols; j++) {
             if (t->table[i][j] == letter) {
                 return (i + 1) * 10 + (j + 1); // Convert to 2-digit format
             }
         }
     }
     return -1; // Return -1 if character isn't found
 }
 
 // Retrieves the letter from the table based on a 2-digit index
 char getLetter(Table *t, int index) {
     int row = (index / 10) - 1; // Extract row from index
     int col = (index % 10) - 1; // Extract column from index
     if (row >= 0 && row < t->rows && col >= 0 && col < t->cols) {
         return t->table[row][col]; // Return the letter at the given position
     }
     return '\0'; // Return null character if invalid index
 }
 
 // Encodes a plaintext string using the Polybius square
 void pbEncode(Table *table, const char *plaintext, char *encoded) {
     int index = 0; // Index for encoded string
     for (size_t i = 0; i < strlen(plaintext); i++) {
         char c = toupper(plaintext[i]); // Convert to uppercase
 
         // Check if character is valid for encoding
         if ((c >= 'A' && c <= 'Z') || c == '$' || c == '.' || c == '?' || c == '!' || 
             c == ',' || c == ';' || c == ':' || c == '\'' || c == '"' || c == '-') {
             
             if (c == 'J') { // Replace 'J' with 'I'
                 c = 'I';
             }
 
             if (c == ' ') { // Replace spaces with '$'
                 c = '$';
             }
 
             int code = getIndices(table, c); // Get the 2-digit code
             if (code != -1) {
                 encoded[index++] = '0' + (code / 10); // Add the row digit
                 encoded[index++] = '0' + (code % 10); // Add the column digit
             }
         }
     }
     encoded[index] = '\0'; // Null-terminate the encoded string
 }
 
 // Decodes a ciphertext string back into plaintext
 void pbDecode(const char *ciphertext, Table *table, char *decoded) {
     int index = 0; // Index for decoded string
     for (size_t i = 0; i < strlen(ciphertext); i += 2) {
         if (i + 1 < strlen(ciphertext)) {
             int row = ciphertext[i] - '0'; // Convert first digit to row
             int col = ciphertext[i + 1] - '0'; // Convert second digit to col
             int code = row * 10 + col; // Combine into a single code
             char letter = getLetter(table, code); // Get the letter from the code
             if (letter != '\0') {
                 decoded[index++] = letter; // Add the letter to the decoded string
             }
         }
     }
     decoded[index] = '\0'; // Null-terminate the decoded string
 }