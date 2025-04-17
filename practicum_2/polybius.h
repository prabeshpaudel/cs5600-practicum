/*
 * polybius.h / Assignment / Random Number Generation and Encryption
 *
 * Prabesh Paudel / CS5600 / Northeastern University
 * Spring 2024 / Jan 24, 2024
 *
 */

 #ifndef POLYBIUS_H
 #define POLYBIUS_H
 
 #include <stddef.h> 
 
 #define TABLE_ROWS 6 // Fixed rows for the encryption table
 #define TABLE_COLS 6 // Fixed cols for the encryption table
 
 // Struct for the table used in encryption
 typedef struct {
     int rows;                        // Number of rows in the table
     int cols;                        // Number of columns in the table
     char table[TABLE_ROWS][TABLE_COLS]; // 2D char array for the table
 } Table;
 
 // Function to initialize the Polybius square (returns a Table)
 Table initializeMatrix();
 
 // Gets the index of a given letter in the table
 int getIndices(Table *t, char letter);
 
 // Gets the letter at a given index from the table
 char getLetter(Table *t, int index);
 
 // Encodes a plaintext string using the Polybius table
 void pbEncode(Table *table, const char *plaintext, char *encoded);
 
 // Decodes an encoded string back to plaintext using the table
 void pbDecode(const char *ciphertext, Table *table, char *decoded);
 
 #endif
 