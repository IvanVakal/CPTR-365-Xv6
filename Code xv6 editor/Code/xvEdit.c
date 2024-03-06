/*
xvEdit.c- simple line-oriented command-based in-memory text editor that runs within the limited xv6 environment.
Ivan Vakal & CPTR-365
1/29/2024
*/

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include <stddef.h>

#define MAX_LINE_LENGTH 256
#define MAX_LINES 1000

int 
putchar(int c)
{
  return write(1, &c, 1);
}

void flipLines(char *content, int numLines, int startLine, int endLine)
{
  char *start = content;
  char *end = content;

  // Find the end of the content
  while (*end != '\0')
  {
    end++;
  }

  // Reverse the lines within the specified range
  while (startLine < endLine && startLine <= endLine)
  {
    // Find the start of the current line
    while (startLine > 1 && *start != '\0')
    {
      if (*start == '\n')
      {
        startLine--;
      }
      start++;
    }

    // Find the end of the current line
    while (*end != '\0')
    {
      if (*end == '\n')
      {
        endLine--;
        if (endLine == 0)
        {
          break;
        }
      }
      end++;
    }

    // Swap lines
    while (start < end)
    {
      char temp = *start;
      *start = *end;
      *end = temp;
      start++;
      end--;
    }

    // Move to the next lines
    start++;
    end++;
    startLine++;
    endLine--;
  }
}
int 
countLines(char *filename)
{
  int fc, n;
  int numLines = 0;
  char buf[MAX_LINE_LENGTH];

  fc = open(filename, O_RDONLY);
  if (fc < 0)
  {
    printf("Error: cannot open file %s\n", filename);
    exit(1);
  }

  while ((n = read(fc, buf, sizeof(buf))) > 0)
  {
    for (int i = 0; i < n; i++)
    {
      if (buf[i] == '\n')
        numLines++;
    }
  }

  close(fc);
  return numLines;
}

void
QUIT(char *filename, char *content, int length)
{
  int fc = open(filename, O_WRONLY | O_CREATE | O_TRUNC);
  if (fc < 0)
  {
    printf("Error: cannot open file %s\n", filename);
    exit(1);
  }

  if (write(fc, content, length) != length)
  {
    printf("Error: cannot write to file %s\n", filename);
    close(fc);
    exit(1);
  }

  close(fc);
}

void 
listLines(char *content, int numLines, int startLine, int endLine)
{
  int currentLine = 1;
  char *start = content;
  char *end = content;

  // Iterate through the content
  while (*end != '\0')
  {
    if (*end == '\n')
    {
      // Check if the current line is within the specified range
      if (currentLine >= startLine && currentLine <= endLine)
      {
        // Print the line
        printf("%d: ", currentLine);
        while (start < end)
        {
          putchar(*start);
          start++;
        }
        printf("\n");
      }
      // Move to the next line
      start = end + 1;
      currentLine++;
    }
    end++;

    // Check if we have reached the end of the range
    if (currentLine > endLine)
    {
      break;
    }
  }
}

void 
insertLineBefore(char *content, int *numLines, int line_num, const char *text)
{
  // Calculate the length of the text to be inserted
  int textLength = strlen(text);

  // Find the position in the content where the new line should be inserted
  char *insertPosition = content;
  int currentLine = 1;
  while (*insertPosition != '\0' && currentLine < line_num)
  {
    if (*insertPosition == '\n')
      currentLine++;
    insertPosition++;
  }

  // Calculate the number of bytes to shift down
  int shiftSize = strlen(insertPosition) + 1; // +1 to include the null terminator

  // Calculate the new size of the content
  int newContentSize = strlen(content) + textLength + 1; // +1 for the new line character

  // Check if there is enough space to insert the new line
  if (newContentSize >= MAX_LINE_LENGTH * MAX_LINES)
  {
    printf("Error: Insufficient space to insert new line.\n");
    return;
  }

  // Shift the content down to make room for the new line
  memmove(insertPosition + textLength + 1, insertPosition, shiftSize);

  // Copy the new line text to the insert position
  memcpy(insertPosition, text, textLength);

  // Insert a new line character '\n'
  insertPosition[textLength] = '\n';

  // Update the number of lines
  (*numLines)++;
}

void 
END(char *content, int *numLines, const char *text)
{
  // Find the end of the current content
  char *end = content;
  while (*end != '\0')
  {
    end++;
  }

  // Append the new line text
  int textLength = strlen(text);
  memcpy(end, text, textLength);
  *(end + textLength) = '\n';

  // Update the number of lines
  (*numLines)++;
}

void 
EDIT(char *content, int numLines, int line_num, const char *text)
{
  // Find the start and end of the line to edit
  char *start = content;
  char *end = content;
  int currentLine = 1;
  while (*end != '\0')
  {
    if (*end == '\n')
    {
      if (currentLine == line_num)
      {
        break;
      }
      start = end + 1;
      currentLine++;
    }
    end++;
  }

  // Calculate the length of the existing line
  int lineLength = end - start;

  // Calculate the length of the new text
  int newTextLength = strlen(text);

  // Calculate the difference in length
  int lengthDiff = newTextLength - lineLength;

  // If the new text is longer than the existing line, shift the content down
  if (lengthDiff > 0)
  {
    memmove(end + lengthDiff, end, strlen(end) + 1); // +1 to include the null terminator
  }

  // Copy the new text into place
  memcpy(start, text, newTextLength);
}

void 
DROP(char *content, int *numLines, int startLine, int endLine)
{
  char *start = content;
  char *end = content;

  // Find the start of the range
  int currentLine = 1;
  while (*end != '\0' && currentLine < startLine)
  {
    if (*end == '\n')
      currentLine++;
    end++;
  }

  // Delete the lines within the range
  while (*end != '\0' && currentLine <= endLine)
  {
    // Move to the next line
    char *nextLineStart = end;
    while (*nextLineStart != '\n' && *nextLineStart != '\0')
    {
      nextLineStart++;
    }
    nextLineStart++; // Move past the newline character

    // Calculate the number of bytes to shift up
    int shiftSize = strlen(nextLineStart) + 1; // +1 to include the null terminator

    // Shift the content up to overwrite the current line
    memmove(start, nextLineStart, shiftSize);

    // Update the end pointer
    end = start;

    // Update the number of lines
    (*numLines)--;

    // Move to the next line
    while (*end != '\n' && *end != '\0')
    {
      end++;
    }
    if (*end == '\n')
    {
      end++; // Move past the newline character
    }

    currentLine++;
  }
}
int 
main(int argc, char *argv[])
{
  // Checks the right typed form of the command xvEdit
  if (argc != 2)
  {
    printf("Usage: xvEdit <filename>\n");
    exit(1);
  }

  // Welcome Message + Numbers of Lines
  char *filename = argv[1];
  int numLines = countLines(filename);
  printf("Welcome to xvEdit!\n");
  printf("%d lines read from the file.\n", numLines);

  // Dynamically allocate memory for a buffer to hold the content of the file.
  char *fileContent = malloc(MAX_LINE_LENGTH * MAX_LINES);
  if (fileContent == 0)
  {
    printf("Error: cannot allocate memory\n");
    exit(1);
  }

  // Read file content
  int fc = open(filename, O_RDONLY);
  if (fc < 0)
  {
    printf("Error: cannot open file %s\n", filename);
    free(fileContent);
    exit(1);
  }
  // Reading the content of a file into memory.
  int bytesRead = read(fc, fileContent, MAX_LINE_LENGTH * MAX_LINES);
  close(fc);
  if (bytesRead < 0)
  {
    printf("Error: cannot read file %s\n", filename);
    free(fileContent);
    exit(1);
  }

  // Main loop for command processing
  char command[MAX_LINE_LENGTH];
  while (1)
  {
    printf("XvEdit> ");
    gets(command, sizeof(command));

    // Tokenize the command to extract LIST, ADD<, EDIT,DROP @END, FIND, or QUIT command and its parameters
    char *token = command;
    while (*token != '\0' && *token != ' ')
      token++;
    *token = '\0';

    // LIST COMMAND
    if (strcmp(command, "LIST") == 0)
    {
      // Extract the range specification
      char *range = ++token;

      // The colon (:) separating the start and end line numbers
      char *colon = strchr(range, ':');
      if (colon != NULL)
      {
        int startLine, endLine;
        // Case: "2:3"
        if (colon == range)
        {
          // Case: ":3"
          startLine = 1;
        }
        else
        {
          *colon = '\0';
          startLine = atoi(range);
        }
        // Case: "15:""
        if (*(colon + 2) == '\0')
        {
          endLine = numLines;
        }
        else
        {

          endLine = atoi(colon + 1);
        }

        // Output the specified lines
        listLines(fileContent, numLines, startLine, endLine);
      }
      else
      {
        // Check for the case of ":"
        if (range[0] == '\0')
        {
          // Output the entire file
          listLines(fileContent, numLines, 1, numLines);
        }
        else
        {
          // Case: "12"
          int line_num = atoi(range);
          if (line_num <= numLines)
          {
            // Output the specified line
            listLines(fileContent, numLines, line_num, line_num);
          }
          else
          {

            listLines(fileContent, numLines, line_num, numLines);
          }
        }
      }
    }
    // ADD COMMAND
    else if (strcmp(command, "ADD<") == 0)
    {
      // The line_num and text
      int line_num = atoi(++token);
      char *text = strchr(++token, ' ');
      if (text == NULL)
      {
        printf("Invalid command syntax for ADD<\n");
        continue;
      }
      *text = '\0'; // Separate line number and text
      text++;

      // Insert the new line containing text before line line_num
      insertLineBefore(fileContent, &numLines, line_num, text);
    }
    // @END COMMAND
    else if (strcmp(command, "@END") == 0)
    {
      // Append text as a new line at the end of the file
      char *text = ++token;
      END(fileContent, &numLines, text);
    }
    // DROP COMMAND
    else if (strcmp(command, "DROP") == 0)
    {
      //  range specification
      char *range = ++token;

      // The colon (:) separating the start and end line numbers
      char *colon = strchr(range, ':');
      if (colon != NULL)
      {
        int startLine, endLine;
        // Case: "3:2"
        if (colon == range)
        {
          // Case: ":14"
          startLine = 1;
        }
        else
        {
          *colon = '\0';
          startLine = atoi(range);
        }
        // Case: "15:"
        if (*(colon + 2) == '\0')
        {
          endLine = numLines;
        }
        else
        {
          endLine = atoi(colon + 1);
        }

        // Confirm deletion with the user
        printf("Drop %d lines (y/N)? ", endLine - startLine + 1);
        char response[MAX_LINE_LENGTH];
        gets(response, sizeof(response));
        if (response[0] == 'Y' || response[0] == 'y')
        {
          // Delete the specified lines
          DROP(fileContent, &numLines, startLine, endLine);
          printf("%d lines dropped\n", endLine - startLine + 1);
        }
      }

      else
      {
        printf("Invalid command syntax for DROP\n");
        continue;
      }
    }
    // EDIT COMMAND
    else if (strcmp(command, "EDIT") == 0)
    {
      // Extract the line number and text
      int line_num = atoi(++token);
      char *text = strchr(++token, ' ');
      if (text == NULL)
      {
        printf("Invalid command syntax for EDIT\n");
        continue;
      }
      *text = '\0';
      text++;

      // Edit the specified line with the new text
      EDIT(fileContent, numLines, line_num, text);
    }
    // HELP COMMAND
    else if (strcmp(command, "HELP") == 0)
    {
      printf("Commands:\n");
      printf("LIST [start:end] - Output lines in range , each preceded by its line number (rightaligned), a colon, and a space\n");
      printf("   Examples:\n");
      printf("     LIST 1:20 - Display lines 1 to 20.\n");
      printf("     LIST :30    - Display lines from the beginning to line 30.\n");
      printf("     LIST 40:    - Display lines from line 40 to the end.\n");
      printf("     LIST        - Display the whole file.\n");
      printf("ADD< <line_num> <text> - Insert text at the specified line number.\n");
      printf("   Example: ADD< 5 This is my text that I want to instert.\n");
      printf("@END <text> - Append text as a new line at the end of the file.\n");
      printf("   Example: @END Text.\n");
      printf("EDIT <line_num> <text> - Replace text at the specified line number.\n");
      printf("   Example: EDIT 3, Text.\n");
      printf("DROP [start:end] - Delete lines in range.\n");
      printf("   Example: DROP 2:5 - Delete lines from 2 to 5.\n");
      printf("QUIT - Exit the editor and optionally save changes.\n");
      printf("   Example: QUIT\n");
      printf("HELP - Display documentation for commands.\n");
      printf("   Example: HELP\n");
    }
    // QUIT COMMAND
    else if (strcmp(command, "QUIT") == 0)
    {
      printf("Save changes (Y/n)? ");
      char response[MAX_LINE_LENGTH];
      gets(response, sizeof(response));
      if (response[0] == 'Y' || response[0] == 'y')
      {
        QUIT
      (filename, fileContent, bytesRead);
        printf("Changes saved.\n");
      }
      free(fileContent);
      exit(0);
    }
    else
    {
      printf("Unknown command\n");
    }
  }

  free(fileContent);
  exit(0);
}
