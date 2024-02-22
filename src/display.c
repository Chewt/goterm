#include <ncurses.h>
#include <string.h>
#include "display.h"
#include "gameinfo.h"
#include "gametree.h"
#include "go.h"

char screen_notes[NOTES_LENGTH];

DisplayConfig displayConfig  = { .centerBoard  = 1,
                                 .showBoard    = 1,
                                 .showInfo     = 1,
                                 .showComments = 1};

DisplayConfig* GetDisplayConfig()
{
    return &displayConfig;
}

char* GetNotes()
{
    return screen_notes;
}

char coords[] = {
    'A', 'B', 'C', 'D', 'E', 'F',
    'G', 'H', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T'};

// Color pair declarations
enum
{
    BLACK_STONE_COLOR = 1,
    WHITE_STONE_COLOR = 2,
    LAST_STONE_COLOR = 3,
};

int BoardFitsScreen(Goban* goban)
{
    GameInfo* gameInfo = GetGameInfo();
    int width_needed = gameInfo->boardSize * 4 + 11;
    int height_needed = gameInfo->boardSize * 2 + 2;
    return (getmaxx(stdscr) >= width_needed) &&
           (getmaxy(stdscr) >= height_needed);
}



void PrintInfo(Goban* goban)
{
    GameInfo* gameInfo = GetGameInfo();
    DisplayConfig* displayConfig = GetDisplayConfig();

    // Determine where to put the game info, accounting for whether or not the
    // board in centered
    int start_xpos = (gameInfo->boardSize + 1) * 4;
    if (displayConfig->centerBoard)
        start_xpos += (getmaxx(stdscr) / 2) - (start_xpos / 2);
    start_xpos += 2;

    // Print Info
    mvprintw(0,start_xpos, "B[%s]: %d", gameInfo->blackName, goban->bpris);
    mvprintw(1,start_xpos, "W[%s]: %d", gameInfo->whiteName, goban->wpris);
    if (gameInfo->result[0] != '\0')
        mvprintw(2,start_xpos, "Result: %s", gameInfo->result);
    char lastmove[10] = { 0 };
    if (GetHistorySize() > 1)
    {
        int idx = snprintf(lastmove, 10, "%d. ", GetViewIndex());
        if (goban->lastmove.p.col == -1)
            snprintf(lastmove + idx, 10, "Pass");
        else
        {
            snprintf(lastmove + idx, 10, "%c%d",
                    coords[goban->lastmove.p.col],
                    gameInfo->boardSize - goban->lastmove.p.row);
        }
    }
    mvprintw(4,start_xpos, "%s", lastmove);
}

// Print board to ncurses screen
void PrintBoardw(Goban* goban)
{
    start_color();
    init_pair(BLACK_STONE_COLOR, COLOR_BLACK, COLOR_YELLOW);
    init_pair(WHITE_STONE_COLOR, COLOR_WHITE, COLOR_YELLOW);
    init_pair(LAST_STONE_COLOR, COLOR_CYAN, COLOR_YELLOW);
    int x_pos = 0;
    int y_pos = 0;

    // Determine where the board should be placed on screen
    GameInfo* gameInfo = GetGameInfo();
    DisplayConfig* displayConfig = GetDisplayConfig();
    int start_xpos = 0;
    if (displayConfig->centerBoard == 1)
    {
        int width_needed = gameInfo->boardSize * 4;
        int max_x = getmaxx(stdscr);
        if ((max_x / 2) >= (width_needed / 2))
            start_xpos = (max_x / 2) - (width_needed / 2);
    }

    clear(); // Set screen to blank

    // Print empty board
    attron(COLOR_PAIR(BLACK_STONE_COLOR));
    int i;
    //    Background color
    for (i = 0; i < (gameInfo->boardSize * 2) + 1; ++i) 
        mvprintw(i, start_xpos, "   %*s", gameInfo->boardSize * 4, ""); // padding abuse
    x_pos = start_xpos + 3;
    y_pos = 1;
    move(y_pos, x_pos);
    //    Top grid
    addch(ACS_ULCORNER);
    addch(ACS_HLINE);
    addch(ACS_HLINE);
    addch(ACS_HLINE);
    for (i = 0; i < gameInfo->boardSize - 2; ++i)
    {
        addch(ACS_TTEE);
        addch(ACS_HLINE);
        addch(ACS_HLINE);
        addch(ACS_HLINE);
    }
    addch(ACS_URCORNER);
    //    Middle Grid
    y_pos++;
    move(y_pos, x_pos);
    int j;
    for (i = 0; i < gameInfo->boardSize - 2; ++i)
    {
        for (j = 0; j < gameInfo->boardSize; ++j)
        {
            addch(ACS_VLINE);
            printw("   ");
        }
        move(++y_pos, x_pos);
        addch(ACS_LTEE);
        addch(ACS_HLINE);
        addch(ACS_HLINE);
        addch(ACS_HLINE);
        for (j = 0; j < gameInfo->boardSize - 2; ++j)
        {
            addch(ACS_PLUS);
            addch(ACS_HLINE);
            addch(ACS_HLINE);
            addch(ACS_HLINE);
        }
        addch(ACS_RTEE);
        move(++y_pos, x_pos);
    }
    for (i = 0; i < gameInfo->boardSize; ++i)
    {
        addch(ACS_VLINE);
        printw("   ");
    }
    //    Bottom Grid
    move(++y_pos, x_pos);
    addch(ACS_LLCORNER);
    addch(ACS_HLINE);
    addch(ACS_HLINE);
    addch(ACS_HLINE);
    for (i = 0; i < gameInfo->boardSize - 2; ++i)
    {
        addch(ACS_BTEE);
        addch(ACS_HLINE);
        addch(ACS_HLINE);
        addch(ACS_HLINE);
    }
    addch(ACS_LRCORNER);

    // Star points or showing score on the board
    const char STARPOINT[] = "\u254b"; // ╋
    if (goban->showscore)
    {
        for (i = 0; i < gameInfo->boardSize; ++i)
        {
            for (j = 0; j < gameInfo->boardSize; ++j)
            {
                if (goban->score[i][j] == 'b' || goban->score[i][j] == 'w')
                {
                    move((i * 2) + 1, (j * 4) + 3 + start_xpos);
                    if (goban->score[i][j] == 'w')
                    {
                        attroff(COLOR_PAIR(BLACK_STONE_COLOR));
                        attrset(COLOR_PAIR(WHITE_STONE_COLOR));
                        attron(A_BOLD);
                    }
                    addstr(STARPOINT); 
                    attrset(COLOR_PAIR(BLACK_STONE_COLOR));
                }
            }
        }
        goban->showscore = 0;
    }
    else if (gameInfo->boardSize == 19)
    {
        mvaddstr(3 * 2 + 1, 3   * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(3 * 2 + 1, 9   * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(3 * 2 + 1, 15  * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(9 * 2 + 1, 3   * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(9 * 2 + 1, 9   * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(9 * 2 + 1, 15  * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(15 * 2 + 1, 3  * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(15 * 2 + 1, 9  * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(15 * 2 + 1, 15 * 4 + 3 + start_xpos, STARPOINT);
    }
    else if (gameInfo->boardSize == 13)
    {
        mvaddstr(3 * 2 + 1, 3 * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(3 * 2 + 1, 6 * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(3 * 2 + 1, 9 * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(6 * 2 + 1, 3 * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(6 * 2 + 1, 6 * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(6 * 2 + 1, 9 * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(9 * 2 + 1, 3 * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(9 * 2 + 1, 6 * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(9 * 2 + 1, 9 * 4 + 3 + start_xpos, STARPOINT);
    }
    else if (gameInfo->boardSize == 9)
    {
        mvaddstr(2 * 2 + 1, 2 * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(2 * 2 + 1, 6 * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(6 * 2 + 1, 2 * 4 + 3 + start_xpos, STARPOINT);
        mvaddstr(6 * 2 + 1, 6 * 4 + 3 + start_xpos, STARPOINT);
    }

    // Print coordinates
    move(0, 3 + start_xpos); 
    for (i = 0; i < gameInfo->boardSize; ++i)
        printw("%c   ", coords[i]); 
    move((gameInfo->boardSize * 2), 3 + start_xpos);
    for (i = 0; i < gameInfo->boardSize; ++i)
        printw("%c   ", coords[i]);
    x_pos = start_xpos;
    y_pos = 1;
    for (i = 0; i < gameInfo->boardSize ; ++i)
    {
        move(y_pos, x_pos);
        printw("%2d", gameInfo->boardSize - i);
        move(y_pos, x_pos + 3 + ((gameInfo->boardSize - 1) * 4) + 2);
        printw("%2d", gameInfo->boardSize - i);
        y_pos += 2;
    }

    // Place stones
    for (i = 0; i < gameInfo->boardSize; ++i)
    {
        for (j = 0; j < gameInfo->boardSize; ++j)
        {
            if (goban->board[i][j] != ' ')
            {
                move((i * 2) + 1, (j * 4) + 2 + start_xpos);
                if (goban->board[i][j] == 'w')
                {
                    attroff(COLOR_PAIR(BLACK_STONE_COLOR));
                    attrset(COLOR_PAIR(WHITE_STONE_COLOR));
                    attron(A_BOLD);
                }
                addstr("\u2588\u2588"); // Block char

                if (goban->lastmove.p.col == j && goban->lastmove.p.row == i)
                    attrset(COLOR_PAIR(LAST_STONE_COLOR));
                addstr("\u2588");
                attrset(COLOR_PAIR(BLACK_STONE_COLOR));
            }
        }
    }
    attroff(COLOR_PAIR(BLACK_STONE_COLOR));
}

int WordSize(char* buf)
{
    int i = 0;
    int count = 0;
    char c;
    while (((c = buf[i++]) != ' ') && (c != '\0') && (c != '\n'))
        count++;
    return count;
}

void PrintComments()
{
    GameInfo* gameInfo = GetGameInfo();
    DisplayConfig* displayConfig = GetDisplayConfig();

    // Determine where to put the comments, accounting for whether or not the
    // board in centered
    int start_ypos = 6;
    int screen_end = getmaxx(stdscr);
    int start_xpos = (gameInfo->boardSize + 1) * 4;
    if (displayConfig->centerBoard)
        start_xpos += (getmaxx(stdscr) / 2) - (start_xpos / 2);
    start_xpos += 2;

    GameNode* current_node = GetViewedNode();
    char c;
    int i = 0;
    int x = start_xpos;
    int y = start_ypos;
    while ((c = current_node->comment[i++]) != '\0')
    {
        if (c == '\n')
        {
            move(++y, x = start_xpos);
            continue;
        }
        else if ((x + WordSize(current_node->comment + i - 1)) > screen_end)
            move(++y, x = start_xpos);
        mvaddch(y, x++, c);
    }
}

void PrintDisplay(Goban* goban)
{
    DisplayConfig* displayConfig = GetDisplayConfig();
    GameInfo* gameInfo = GetGameInfo();
    clear();
    if (displayConfig->showBoard)
        PrintBoardw(goban);
    if (displayConfig->showInfo)
        PrintInfo(goban);
    if (displayConfig->showComments)
        PrintComments();
    move(gameInfo->boardSize * 2 + 1, 0);
}

void PrintNotesw(Goban* goban)
{
    // Count how many lines the notes have
    char c;
    int i = 0;
    int linecount = 0;
    while ((c = screen_notes[i++]) != '\0')
    {
        if (c == '\n')
            linecount++;
    }

    // Print notes and adjust starting y for notes
    int x, y;
    x = 0;
    y = getcury(stdscr);
    if ((getmaxy(stdscr) - y) <= linecount)
        y -= linecount;
    i = 0;
    while ((c = screen_notes[i++]) != '\0')
    {
        if (c == '\n')
            move(++y, x = 0);
        else
            mvaddch(y, x++, c);
    }

    // Reset notes to empty
    screen_notes[0] = '\0';
}

// Print board to screen
void PrintBoard(Goban* goban)
{
    GameInfo* gameInfo = GetGameInfo();
    printf("\e[2J\e[H"); // Clear Screen and position cursor top left
    int i, j;
    printf("\e[30;43m ");
    for (i = 0; i < gameInfo->boardSize; ++i)
        printf("  %c ", coords[i]);
    printf("  \e[0m B: %d\n", goban->bpris);
    for (i = 0; i < (2 * gameInfo->boardSize) - 1; ++i)
    {
        for (j = 0; j < gameInfo->boardSize; ++j)
        {
            printf("\e[30;43m"); // Reset terminal colors
            if (goban->board[i/2][j] == ' ' || (i & 0x1))
            {
                if (i & 0x1)
                {
                    if (j == 0)
                        printf("  ");
                    printf(" \u2502"); // │
                    if (j == gameInfo->boardSize - 1)
                        printf("   \e[0m\n");
                    else
                        printf("  ");
                }
                else if (i == 0)
                {
                    if (j == 0){
                        printf("%2d \e[30;43m", gameInfo->boardSize);
                        if (goban->showscore && ((goban->score[i][j] == 'w') ||
                                    (goban->score[i/2][j] == 'b'))) 
                        {
                            if (goban->score[i/2][j] == 'w')
                                printf("\e[97;43m");
                            printf("\u254b"); // ╋
                            printf("\e[30;43m");
                        }
                        else
                            printf("\u250c"); // ┌
                        printf("\u2500\u2500"); // ──
                    }
                    else if (j == gameInfo->boardSize - 1){ // 
                        printf("\u2500"); // ─
                        if (goban->showscore && ((goban->score[i/2][j] == 'w') ||
                                    (goban->score[i/2][j] == 'b'))) 
                        {
                            if (goban->score[i][j] == 'w')
                                printf("\e[97;43m");
                            printf("\u254b"); // ╋
                            printf("\e[30;43m");
                        }
                        else
                            printf("\u2510"); // ┐
                        printf(" %2d\e[0m W: %d\n", gameInfo->boardSize,
                                goban->wpris);
                    }
                    else
                    {
                        printf("\u2500"); // ─
                        if (goban->showscore && ((goban->score[i/2][j] == 'w') ||
                                    (goban->score[i/2][j] == 'b'))) 
                        {
                            if (goban->score[i/2][j] == 'w')
                                printf("\e[97;43m");
                            printf("\u254b"); // ╋
                            printf("\e[30;43m");
                        }
                        else
                            printf("\u252c"); // ┬
                        printf("\u2500\u2500"); // ──
                    }
                }
                else if (i == (2 * gameInfo->boardSize) - 2)
                {
                    if (j == 0) {

                        printf(" 1\e[30;43m ");
                        if (goban->showscore && ((goban->score[i/2][j] == 'w') ||
                                    (goban->score[i/2][j] == 'b'))) 
                        {
                            if (goban->score[i/2][j] == 'w')
                                printf("\e[97;43m");
                            printf("\u254b"); // ╋
                            printf("\e[30;43m");
                        }
                        else
                            printf("\u2514"); // └
                        printf("\u2500\u2500"); //──
                    }
                    else if (j == gameInfo->boardSize - 1)
                    {
                        printf("\u2500"); // ─
                        if (goban->showscore && ((goban->score[i/2][j] == 'w') ||
                                    (goban->score[i/2][j] == 'b'))) 
                        {
                            if (goban->score[i/2][j] == 'w')
                                printf("\e[97;43m");
                            printf("\u254b"); // ╋
                            printf("\e[30;43m");
                        }
                        else
                            printf("\u2518"); // ┘
                        printf("  1\e[0m\n"); 
                        i++;
                    }
                    else
                    {
                        printf("\u2500"); // ─
                        if (goban->showscore && ((goban->score[i/2][j] == 'w') ||
                                    (goban->score[i/2][j] == 'b'))) 
                        {
                            if (goban->score[i/2][j] == 'w')
                                printf("\e[97;43m");
                            printf("\u254b"); // ╋
                            printf("\e[30;43m");
                        }
                        else
                            printf("\u2534"); // ┴
                        printf("\u2500\u2500"); // ──
                    }
                }
                else
                {
                    if (j == 0)
                    {
                        printf("%2d \e[30;43m", gameInfo->boardSize - (i / 2));
                        if (goban->showscore && ((goban->score[i/2][j] == 'w') ||
                                    (goban->score[i/2][j] == 'b'))) 
                        {
                            if (goban->score[i/2][j] == 'w')
                                printf("\e[97;43m");
                            printf("\u254b"); // ╋
                            printf("\e[30;43m");
                        }
                        else
                            printf("\u251c"); // ├
                        printf("\u2500\u2500"); // ──
                    }
                    else if (j == gameInfo->boardSize - 1)
                    {
                        printf("\u2500"); // ─
                        if (goban->showscore && ((goban->score[i/2][j] == 'w') ||
                                    (goban->score[i/2][j] == 'b'))) 
                        {
                            if (goban->score[i/2][j] == 'w')
                                printf("\e[97;43m");
                            printf("\u254b"); // ╋
                            printf("\e[30;43m");
                        }
                        else
                            printf("\u2524"); // ┤
                        printf(" %2d\e[0m", gameInfo->boardSize - (i / 2));
                        if (i == 2)
                        {
                            char lastmove[5] = { 0 };
                            if (GetHistorySize() >= 1)
                            {
                                if (goban->lastmove.p.col == -1)
                                    snprintf(lastmove, 5, "Pass");
                                else
                                {
                                    snprintf(lastmove, 5, "%c%d",
                                            coords[goban->lastmove.p.col],
                                            gameInfo->boardSize - goban->lastmove.p.row);
                                }
                            }
                            printf(" %s", lastmove);
                        }
                        printf("\n");
                    }
                    else
                    {
                        printf("\u2500"); // ─
                        if (goban->showscore == 0)
                        {
                            if (gameInfo->boardSize == 19 &&
                                    (i == 6 || i == 18 || i == 30) &&
                                    (j == 3 || j == 9  || j == 15))
                                printf("\u254b"); // ╋
                            else if (gameInfo->boardSize == 13 && 
                                    (i == 6 || i == 12 || i == 18) &&
                                    (j == 3 || j == 6  || j == 9 ))
                                printf("\u254b"); // ╋
                            else if (gameInfo->boardSize == 9 && 
                                    (i == 4 || i == 12) &&
                                    (j == 2 || j == 6))
                                printf("\u254b"); // ╋
                            else printf("\u253c"); // ┼
                        }
                        else
                        {
                            if ((goban->score[i/2][j] == 'w') ||
                                    (goban->score[i/2][j] == 'b'))
                            {
                                if (goban->score[i/2][j] == 'w')
                                    printf("\e[97;43m");
                                printf("\u254b"); // ╋
                                printf("\e[30;43m");
                            }
                            else
                                printf("\u253c"); // ┼
                        }
                        printf("\u2500\u2500"); // ──
                    }
                }
            }
            else
            {
                if (j == 0)
                    printf("%2d", gameInfo->boardSize - (i/2));
                if (goban->board[i/2][j] == 'w')
                    printf("\e[97;43m");
                printf("\u2588\u2588"); // █
                if (goban->lastmove.p.col == j && goban->lastmove.p.row == i/2)
                    printf("\e[36m");
                printf("\u2588");
                if (j == gameInfo->boardSize - 1)
                {
                    printf("\e[30;43m%2d\e[0m ", gameInfo->boardSize - (i/2));
                    if (i == 0)
                        printf("W: %d", goban->wpris);
                    printf("\n");
                }
                else
                    printf("\e[0m\e[30;43m\u2500");
            }
        }
    }
    printf("\e[30;43m ");
    for (i = 0; i < gameInfo->boardSize; ++i)
        printf("  %c ", coords[i]);
    goban->showscore = 0;
    printf("  \e[0m\n");
}
