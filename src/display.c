#include <ncurses.h>
#include <stdio.h>
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
                                 .showComments  = 1,
                                 .showLabels    = 1};

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
    int y = 0;
    char name_and_rank[NAME_LENGTH + RANK_LENGTH + 4];

    // Set up black name
    int idx = snprintf(name_and_rank, NAME_LENGTH, "B[%s", gameInfo->blackName);
    if (gameInfo->blackRank[0])
        idx += snprintf(name_and_rank + idx, RANK_LENGTH, " %s", gameInfo->blackRank);
    snprintf(name_and_rank + idx, 2, "]");

    // Print black name and captures
    mvprintw(y++,start_xpos, "%s", name_and_rank);
    mvprintw(y++,start_xpos, "Captures: %d", goban->wpris);

    // Set up white name
    idx = snprintf(name_and_rank, NAME_LENGTH, "W[%s", gameInfo->whiteName);
    if (gameInfo->whiteRank[0])
        idx += snprintf(name_and_rank + idx, RANK_LENGTH, " %s", gameInfo->whiteRank);
    snprintf(name_and_rank + idx, 2, "]");

    // Print white name and captures
    mvprintw(y++,start_xpos, "%s", name_and_rank);
    mvprintw(y++,start_xpos, "Captures: %d", goban->bpris);

    // Write result
    if (gameInfo->result[0] != '\0')
        mvprintw(++y,start_xpos, "Result: %s", gameInfo->result);
    char lastmove[16] = { 0 };
    if (GetHistorySize() > 1)
    {
        int idx = snprintf(lastmove, 16, "Last: ");
        if (goban->lastmove.p.col == -1)
            snprintf(lastmove + idx, 16, "Pass");
        else
        {
            snprintf(lastmove + idx, 10, "%c%d",
                    coords[goban->lastmove.p.col],
                    gameInfo->boardSize - goban->lastmove.p.row);
        }
    }
    y += 2;
    mvprintw(y,start_xpos, "%s", lastmove);
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
            if (node->labels[row][col] == ' ')
            {
                node->labels[row][col] = '+';
                for (i = 0; i < node->n_alts; ++i)
                {
                    col = node->alts[i]->goban.lastmove.p.col;
                    row = node->alts[i]->goban.lastmove.p.row;
                    node->labels[row][col] = '@';
                }
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
            /*else if (goban->board[i][j] != ' ')*/
            /*{*/
            /*    move((i * 2) + 1, (j * 4) + 2 + start_xpos);*/
            /*    attron(A_BOLD);*/
            /*    addch(ACS_HLINE);*/
            /*    addch(goban->board[i][j]);*/
            /*    addch(ACS_HLINE);*/
            /*    attroff(A_BOLD);*/
            /*}*/
        }
    }

    // Show labels
    if (displayConfig->showLabels)
    {
        GameNode* node = GetViewedNode();
        int i, j;
        for (i = 0; i < 19; ++i)
            for (j = 0; j < 19; ++j)
            {
                if (node->labels[i][j] != ' ')
                {
                    if (goban->board[i][j] == ' ')
                    {
                        move((i * 2) + 1, (j * 4) + 2 + start_xpos);
                        attron(A_BOLD);
                        addch(ACS_HLINE);
                        addch(node->labels[i][j]);
                        addch(ACS_HLINE);
                        attroff(A_BOLD);
                    }
                    else
                    {
                        move((i * 2) + 1, (j * 4) + 2 + start_xpos + 1);
                        attron(A_BOLD);
                        addch(node->labels[i][j]);
                        attroff(A_BOLD);
                    }
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
    // board is centered
    int start_ypos = 10;
    int screen_end = getmaxx(stdscr);
    int start_xpos = (gameInfo->boardSize + 1) * 4;
    if (displayConfig->centerBoard)
        start_xpos += (getmaxx(stdscr) / 2) - (start_xpos / 2);
    start_xpos += 2;

    int label_placement = start_xpos + ((screen_end - start_xpos) / 2) - (strlen("Comment") / 2);

    int i;
    attron(A_BOLD);
    mvaddch(start_ypos - 1, start_xpos - 1, ACS_ULCORNER);
    for (i = start_xpos; i < label_placement; ++i)
        mvaddch(start_ypos - 1, i, ACS_HLINE);
    mvaddstr(start_ypos - 1, label_placement, "Comment");
    for (i = label_placement + strlen("Comment"); i < screen_end; ++i)
        mvaddch(start_ypos - 1, i, ACS_HLINE);
    attroff(A_BOLD);

    GameNode* current_node = GetViewedNode();
    char c;
    i = 0;
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

int CountLongestBranch(GameNode* node)
{
    int longest_branch_count = 0;
    int i;
    for (i = 0; i < node->n_alts; ++i)
    {
        int count = CountNodes(node->alts[i]);
        if (count > longest_branch_count)
            longest_branch_count = count;
    }
    return longest_branch_count;
}

int CountUntilNextBranch(GameNode* node)
{
    int until_next_branch = 1;
    while (node->mainline_next && !node->mainline_next->n_alts)
    {
        node = node->mainline_next;
        until_next_branch++;
    }
    return until_next_branch;
}

GameNode* FindNextBranch(GameNode* base)
{
    GameNode* node = base; 
    while (node->mainline_next && !node->mainline_next->n_alts)
    {
        node = node->mainline_next;
    }
    return node->mainline_next;
}

int DetermineBranchPrintHeight(GameNode* base, int limit)
{
    if (!base || limit <= 0)
        return 0;
    int longest_branch_count = CountLongestBranch(base);
    int until_next_branch = CountUntilNextBranch(base);
    if (longest_branch_count < until_next_branch)
        return 0;
    GameNode* next_branch = FindNextBranch(base);
    if (!next_branch)
        return 0;
    int height = next_branch->n_alts;
    height += DetermineBranchPrintHeight(next_branch, limit - until_next_branch);
    return height;
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
    {
        limit--;
        mvaddch(y, x++, '<');
    }

    int count = 0;
    while (node->mainline_next)
    {
        count++;
        node = node->mainline_next;
    }

    // Get number of branches past this point
    //int branches_after = CountBranches(base->mainline_next, limit);

    if (!skip)
        limit--;

    node = base;
    if (node->n_alts)
    {
        int i;
        int branch_height = DetermineBranchPrintHeight(node, limit);
        for (i = 0; i < branch_height && !skip; ++i)
            mvaddch(y + i + 1, x, '|');

        for (i = 0; i < node->n_alts; ++i)
        {
            if (!skip)
                mvaddch(y + branch_height + i + 1, x, '\\');

            move(y + branch_height + i + 1, (!skip) ? x + 1 : x);
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
    if (node->mainline_next == NULL && (count < limit))
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

void PrintTreeModule()
{
    GameInfo* gameInfo = GetGameInfo();
    DisplayConfig* displayConfig = GetDisplayConfig();
    int start_xpos = 0;
    int tree_width = (getmaxx(stdscr) / 2) - ((gameInfo->boardSize * 4) / 2) - 1;
    if (!displayConfig->centerBoard)
        tree_width = 0;
    int start_ypos = 10;
    char move_number[10];
    snprintf(move_number, 10, "Move %d", GetViewIndex());
    int move_placement = start_xpos + ((tree_width / 2) - (strlen(move_number) / 2));
    int i;
    attron(A_BOLD);
    for (i = start_xpos; i <= move_placement; ++i)
        mvaddch(start_ypos - 1, start_xpos + i, ACS_HLINE);
    mvaddstr(start_ypos - 1, move_placement, move_number);
    for (i = move_placement + strlen(move_number); i < tree_width; ++i)
        mvaddch(start_ypos - 1, start_xpos + i, ACS_HLINE);
    mvaddch(start_ypos - 1, start_xpos + i, ACS_URCORNER);
    attroff(A_BOLD);
    move(start_ypos, start_xpos);
    PrintTree(GetRootNode(), GetViewIndex() - (tree_width / 4), tree_width);
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
        PrintTreeModule();
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
