#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include "../ThirdParty/raygui.h"

int cursorY = 150;
int cursorX = 0;

void deleteText(char *text, int textLength, int codepointSize) {
  // Delete codepoint from text, before current cursor position
  if ((textLength > 0) &&
      (IsKeyPressed(KEY_BACKSPACE) ||
       (IsKeyDown(KEY_BACKSPACE) &&
        (autoCursorCooldownCounter >= RAYGUI_TEXTBOX_AUTO_CURSOR_COOLDOWN)))) {
    autoCursorDelayCounter++;

    if (IsKeyPressed(KEY_BACKSPACE) ||
        (autoCursorDelayCounter % RAYGUI_TEXTBOX_AUTO_CURSOR_DELAY) ==
            0) // Delay every movement some frames
    {
      int prevCodepointSize = 0;
      GetCodepointPrevious(text + textBoxCursorIndex, &prevCodepointSize);

      // Move backward text from cursor position
      for (int i = (textBoxCursorIndex - prevCodepointSize); i < textLength;
           i++)
        text[i] = text[i + prevCodepointSize];

      // Prevent cursor index from decrementing past 0
      if (textBoxCursorIndex > 0) {
        textBoxCursorIndex -= codepointSize;
        textLength -= codepointSize;
      }

      // Make sure text last character is EOL
      text[textLength] = '\0';
    }
  }
}

void moveCursorHorizontal(char *text, int textLength, int codepointSize) {
  // Move cursor position with keys
  if (IsKeyPressed(KEY_LEFT) ||
      (IsKeyDown(KEY_LEFT) && cursorX > 0 &&
       (autoCursorCooldownCounter > RAYGUI_TEXTBOX_AUTO_CURSOR_COOLDOWN))) {
    autoCursorDelayCounter++;

    int prevCodepointSize = 0;
    GetCodepointPrevious(text + textBoxCursorIndex, &prevCodepointSize);

    cursorX -= GuiGetStyle(DEFAULT, TEXT_SIZE);
    if (textBoxCursorIndex >= prevCodepointSize)
      textBoxCursorIndex -= prevCodepointSize;
  } else if (IsKeyPressed(KEY_RIGHT) ||
             (IsKeyDown(KEY_RIGHT) && (autoCursorCooldownCounter >
                                       RAYGUI_TEXTBOX_AUTO_CURSOR_COOLDOWN))) {
    autoCursorDelayCounter++;
    cursorX += GuiGetStyle(DEFAULT, TEXT_SIZE) * 0.8;

    int nextCodepointSize = 0;
    GetCodepointNext(text + textBoxCursorIndex, &nextCodepointSize);

    if ((textBoxCursorIndex + nextCodepointSize) <= textLength)
      textBoxCursorIndex += nextCodepointSize;
  }
}

int textwidth(char *text, int start, int end) {
  int width = 0;
  for (int i = start; i < end; i++) {
    char c = text[i];
    width += GetTextWidth(&c);
  }
  return width;
}

void moveCursorVertical(Rectangle cursor, char *text) {
  int nextNewLine = 0;
  int prevNewLine = 0;
  int textLength = (int)strlen(text);

  int textWidth = 0;
  for (int i = textBoxCursorIndex; i < textLength; i++) {
    if (text[i] == '\n') {
      textWidth += GetTextWidth(&text[i]);
      nextNewLine = i;
      break;
    }
  }

  for (int i = textBoxCursorIndex; i > 0; i--) {
    if (text[i] == '\n') {
      prevNewLine = i;
      break;
    }
  }

  if (IsKeyPressed(KEY_UP)) {
    if (prevNewLine == 0) {
      return;
    }
    cursorY -= GuiGetStyle(DEFAULT, TEXT_SIZE);
    textBoxCursorIndex = prevNewLine + (textBoxCursorIndex - nextNewLine);
    cursorX = textwidth(text, prevNewLine, textBoxCursorIndex);
  } else if (IsKeyPressed(KEY_DOWN)) {
    if (nextNewLine == 0) {
      return;
    }
    textBoxCursorIndex = nextNewLine + (textBoxCursorIndex - prevNewLine);
    cursorX = textWidth;
    printf("Cursor Y: %d\n", cursorY);
    printf("Cursof X: %d\n", cursorX);
    cursorY += GuiGetStyle(DEFAULT, TEXT_SIZE);
  }
}

int JIBGuiTextBox(Rectangle bounds, char *text, int bufferSize) {
  int result = 0;
  GuiState state = guiState;

  bool multiline = true; // TODO: Consider multiline text input
  int wrapMode = GuiGetStyle(DEFAULT, TEXT_WRAP_MODE);

  Rectangle textBounds = GetTextBounds(TEXTBOX, bounds);
  int textWidth = GetTextWidth(text) - GetTextWidth(text + textBoxCursorIndex);
  int textIndexOffset = 0; // Text index offset to start drawing in the box

  // Cursor rectangle
  // NOTE: Position X value should be updated
  Rectangle cursor = {textBounds.x + cursorX +
                          GuiGetStyle(DEFAULT, TEXT_SPACING),
                      textBounds.y + cursorY - GuiGetStyle(DEFAULT, TEXT_SIZE),
                      2, (float)GuiGetStyle(DEFAULT, TEXT_SIZE)};
  // Auto-cursor movement logic
  // NOTE: Cursor moves automatically when key down after some time
  if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_UP) ||
      IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_BACKSPACE) || IsKeyDown(KEY_DELETE))
    autoCursorCooldownCounter++;
  else {
    autoCursorCooldownCounter = 0; // GLOBAL: Cursor cooldown counter
    autoCursorDelayCounter = 0;    // GLOBAL: Cursor delay counter
  }

  // Update control
  //--------------------------------------------------------------------
  state = STATE_PRESSED;

  // If text does not fit in the textbox and current cursor position is out
  // of bounds, we add an index offset to text for drawing only what
  // requires depending on cursor

  int textLength = (int)strlen(text); // Get current text length
  int codepoint = GetCharPressed();   // Get Unicode codepoint
  if (IsKeyPressed(KEY_ENTER))
    codepoint = (int)'\n';

  if (textBoxCursorIndex > textLength)
    textBoxCursorIndex = textLength;

  // Encode codepoint as UTF-8
  int codepointSize = 0;
  const char *charEncoded = CodepointToUTF8(codepoint, &codepointSize);

  // Add codepoint to text, at current cursor position
  // NOTE: Make sure we do not overflow buffer size
  if ((codepoint == (int)'\n' || (codepoint >= 32)) &&
      ((textLength + codepointSize) < bufferSize)) {
    // Move forward data from cursor position
    for (int i = (textLength + codepointSize); i > textBoxCursorIndex; i--)
      text[i] = text[i - codepointSize];

    // Add new codepoint in current cursor position
    for (int i = 0; i < codepointSize; i++)
      text[textBoxCursorIndex + i] = charEncoded[i];

    textBoxCursorIndex += codepointSize;
    textLength += codepointSize;
    cursorX += GetTextWidth(charEncoded);

    // Make sure text last character is EOL
    text[textLength] = '\0';
  }

  moveCursorVertical(cursor, text);
  moveCursorHorizontal(text, textLength, codepointSize);
  deleteText(text, textLength, codepointSize);
  //--------------------------------------------------------------------

  // Draw control
  //--------------------------------------------------------------------
  // Draw text considering index offset if required
  // NOTE: Text index offset depends on cursor position
  // allign to 0, 0 top left corner

  DrawText(text, 10, 140, GuiGetStyle(DEFAULT, TEXT_SIZE), DARKGRAY);
  // Draw cursor
  GuiDrawRectangle(cursor, 0, BLANK,
                   GetColor(GuiGetStyle(TEXTBOX, BORDER_COLOR_PRESSED)));
  //--------------------------------------------------------------------

  return result; // Mouse button pressed: result = 1
}

int main() {
  // Initialization
  //---------------------------------------------------------------------------------------
  const int screenWidth = 1300;
  const int screenHeight = 600;

  SetConfigFlags(FLAG_WINDOW_UNDECORATED);
  InitWindow(screenWidth, screenHeight, "raygui - portable window");

  // General variables
  Vector2 mousePosition = {0};
  Vector2 windowPosition = {500, 200};
  Vector2 panOffset = mousePosition;
  bool dragWindow = false;

  GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
  GuiSetStyle(DEFAULT, TEXT_SPACING, 2);

  char myText[1024] = "LLLLLLLLLLLLLLLLLLLLLLLLL\nLLLLL"
                      "LLLLLLLLLLLLLLLLLLLLLLLLLLLLLL"
                      "LLLLLLLLLLLLLLLLLLLLLLLLLL\nLLLL"
                      "laborum.";
  SetWindowPosition(windowPosition.x, windowPosition.y);

  bool exitWindow = false;

  SetTargetFPS(60);
  //--------------------------------------------------------------------------------------

  // Main game loop
  while (!exitWindow &&
         !WindowShouldClose()) // Detect window close button or ESC key
  {
    // Update
    //----------------------------------------------------------------------------------
    mousePosition = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !dragWindow) {
      if (CheckCollisionPointRec(mousePosition,
                                 (Rectangle){0, 0, screenWidth, 20})) {
        dragWindow = true;
        panOffset = mousePosition;
      }
    }

    if (dragWindow) {
      windowPosition.x += (mousePosition.x - panOffset.x);
      windowPosition.y += (mousePosition.y - panOffset.y);

      SetWindowPosition((int)windowPosition.x, (int)windowPosition.y);

      if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        dragWindow = false;
    }
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);

    exitWindow = GuiWindowBox((Rectangle){0, 0, screenWidth, screenHeight},
                              "#198# PORTABLE WINDOW");

    DrawText(TextFormat("Mouse Position: [ %.0f, %.0f ]", mousePosition.x,
                        mousePosition.y),
             10, 40, 10, DARKGRAY);
    DrawText(TextFormat("Window Position: [ %.0f, %.0f ]", windowPosition.x,
                        windowPosition.y),
             10, 60, 10, DARKGRAY);

    JIBGuiTextBox((Rectangle){0, 0, screenWidth, screenHeight}, myText, 1024);

    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow(); // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}
