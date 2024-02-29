#include <ncurses.h>
#include <string.h>
#include "display.h"
#include "gameinfo.h"
#include "gametree.h"
#include "go.h"

char screen_notes[NOTES_LENGTH];

DisplayConfig displayConfig  = { .centerBoard   = 1,
                                 .showBoard     = 1,
                                 .showInfo      = 1,
                                 .showNextMoves = 1,
                                 .showTree      = 1,
                                 .showComments  = 1};

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
    int width_needed = gameInfo->boardSize * 4;
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

    // Show next move / variations
    if (displayConfig->showNextMoves)
    {
        GameNode* node = GetViewedNode();
        if (node->mainline_next != NULL)
        {
            int col = node->mainline_next->goban.lastmove.p.col;
            int row = node->mainline_next->goban.lastmove.p.row;
            goban->board[row][col] = 'A';
            for (i = 0; i < node->n_alts; ++i)
            {
                col = node->alts[i]->goban.lastmove.p.col;
                row = node->alts[i]->goban.lastmove.p.row;
                goban->board[row][col] = 'B' + i;
            }
        }
    }

    // Place stones
    for (i = 0; i < gameInfo->boardSize; ++i)
    {
        for (j = 0; j < gameInfo->boardSize; ++j)
        {
            if (goban->board[i][j] == 'w' || goban->board[i][j] == 'b')
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
            else if (goban->board[i][j] != ' ')
            {
                move((i * 2) + 1, (j * 4) + 2 + start_xpos);
                attron(A_BOLD);
                addch(ACS_HLINE);
                addch(goban->board[i][j]);
                addch(ACS_HLINE);
                attroff(A_BOLD);
            }
        }
    }
    attroff(COLOR_PAIR(BLACK_STONE_COLOR));

    // Remove next move / variations markup
    if (displayConfig->showNextMoves)
    {
        GameNode* node = GetViewedNode();
        if (node->mainline_next != NULL)
        {
            int col = node->mainline_next->goban.lastmove.p.col;
            int row = node->mainline_next->goban.lastmove.p.row;
            goban->board[row][col] = ' ';
            for (i = 0; i < node->n_alts; ++i)
            {
                col = node->alts[i]->goban.lastmove.p.col;
                row = node->alts[i]->goban.lastmove.p.row;
                goban->board[row][col] = ' ';
            }
        }
    }
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
    int start_ypos = 15;
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

void PrintTree(GameNode* base, int skip, int limit)
{

    if (skip < 0)
        skip = 0;
    GameNode* node = base;
    int x, y;
    x = getcurx(stdscr);
    y = getcury(stdscr);

    if (base == NULL || limit == 0)
        return;

    if (base == GetRootNode() && !skip)
        mvaddch(y, x++, '<');

    // Get number of branches past this point
    int branches_after = CountBranches(base->mainline_next, limit);

    if (!skip)
        limit--;

    if (node->n_alts)
    {
        int i;

        for (i = 0; i < branches_after && !skip; ++i)
            mvaddch(y + i + 1, x, '|');

        for (i = 0; i < node->n_alts; ++i)
        {
            if (!skip)
                mvaddch(y + branches_after + i + 1, x, '\\');

            move(y + branches_after + i + 1, (!skip) ? x + 1 : x);
            PrintTree(node->alts[i], skip - 1, limit);
        }
    }
    if (!skip)
    {
        if (node == GetViewedNode())
            mvaddch(y, x, '%');
        else
            mvaddch(y, x, '-');
    }
    move(y, (!skip) ? x + 1 : x);
    if (node->mainline_next == NULL)
    {
        node = GetRootNode();
        while (node->mainline_next != NULL)
            node = node->mainline_next;
        if (node == base)
            mvaddch(y, x + 1, '>');
    }
    PrintTree(node->mainline_next, skip - 1, limit);
    return;
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
    if (displayConfig->showTree)
    {
        int start_xpos = (gameInfo->boardSize + 1) * 4;
        if (displayConfig->centerBoard)
            start_xpos += (getmaxx(stdscr) / 2) - (start_xpos / 2);
        start_xpos += 2;
        move(6, start_xpos);
        PrintTree(GetRootNode(), GetViewIndex() - 5, getmaxx(stdscr) - start_xpos - 1);
    }
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
