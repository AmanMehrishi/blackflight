#include <bits/stdc++.h>
#include <iostream>
#include <chrono>
#include <unistd.h>
#include <windows.h>
#define U64 unsigned long long

// DEBUGGING FEN STRINGS
#define EMPTY_FEN "8/8/8/8/8/8/8/8 w - - "
#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
#define TRICKY_FEN "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define KILLER_FEN "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define CMK_FEN "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "

#define ENCODE_MOVE(source, target, piece, promoted, capture, dble, enpass, castling) (source | (target << 6) | (piece << 12) | (promoted << 16) | (capture << 20) | (dble << 21) | (enpass << 22) | (castling << 23))

// EXTRACT MOVE DATA
#define FROMSQ(move) ((move & 0x3F))
#define TOSQ(move) ((move >> 6) & 0x3F)
#define GET_PIECE(move) ((move >> 12) & 0xF)
#define GET_PROMOTED(move) ((move >> 16) & 0xF)
#define GET_CAPTURE(move) ((move & 0x100000))
#define GET_DOUBLE(move) ((move & 0x200000))
#define GET_ENPASSANT(move) ((move & 0x400000))
#define GET_CASTLING(move) ((move & 0x800000))

#define time_MS() chrono::high_resolution_clock::now();

#define COPY_BOARD()                             \
    U64 bitboards_copy[12], occupancies_copy[3]; \
    int castle_copy = castle;                    \
    int enpassant_copy = enpassant;              \
    int side_copy = side;                        \
    memcpy(bitboards_copy, bitboards, 96);       \
    memcpy(occupancies_copy, occupancies, 24);   \
    U64 hash_key_copy = hash_key;                \

#define RESET_BOARD()                          \
    memcpy(bitboards, bitboards_copy, 96);     \
    memcpy(occupancies, occupancies_copy, 24); \
    castle = castle_copy;                      \
    enpassant = enpassant_copy;                \
    side = side_copy;                          \
    hash_key = hash_key_copy;                  \

typedef struct
{
    int moves[256];
    int move_count;
} moves;

const int castling_rights[64] = {
    7, 15, 15, 15, 3, 15, 15, 11,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    13, 15, 15, 15, 12, 15, 15, 14};

static int mvv_lva[12][12] = {
 	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600,

	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600
};
#define MAX_PLY 64
#define Ro 2
#define valWindow 50


#define hash_size 0x400000
#define hash_flag_exact 0
#define hash_flag_alpha 1
#define hash_flag_beta  2
#define no_hash_entry 100000


#define mate_value 49000
#define mate_score 48000
#define inf 50000

typedef struct {
    U64 hash_key;   
    int depth;      
    int flag;      
    int score;  
} tt;    

tt hash_table[hash_size];

int killer_moves[2][MAX_PLY];

int history_moves[12][64];

int pv_length[MAX_PLY];
int pv_table[MAX_PLY][MAX_PLY];
int follow_pv, score_pv;


//FILE MASKS
U64 file_masks[64];

//RANK MASKS
U64 rank_masks[64];

//ISOLATED PAWN MASKS
U64 isolated_masks[64];

//PASSED PAWN MASKS
U64 white_passed_masks[64];
U64 black_passed_masks[64];

U64 piece_keys[12][64];
U64 enpassant_keys[64];

U64 castle_keys[16];
U64 side_key;
// ENUM BOARD SQUARES

enum
{
    a8,
    b8,
    c8,
    d8,
    e8,
    f8,
    g8,
    h8,
    a7,
    b7,
    c7,
    d7,
    e7,
    f7,
    g7,
    h7,
    a6,
    b6,
    c6,
    d6,
    e6,
    f6,
    g6,
    h6,
    a5,
    b5,
    c5,
    d5,
    e5,
    f5,
    g5,
    h5,
    a4,
    b4,
    c4,
    d4,
    e4,
    f4,
    g4,
    h4,
    a3,
    b3,
    c3,
    d3,
    e3,
    f3,
    g3,
    h3,
    a2,
    b2,
    c2,
    d2,
    e2,
    f2,
    g2,
    h2,
    a1,
    b1,
    c1,
    d1,
    e1,
    f1,
    g1,
    h1,
    no_sqr
};
enum
{
    WHITE,
    BLACK,
    BOTH
};

enum
{
    ROOK,
    BISHOP
};

// CASTLE PERMISSIONS
enum
{
    WK = 1,
    WQ = 2,
    BK = 4,
    BQ = 8
};

// PIECE ENCODING
enum
{
    P,
    N,
    B,
    R,
    Q,
    K,
    p,
    n,
    b,
    r,
    q,
    k
};

enum
{
    all_moves,
    captures
};

const char *sq_to_str[64] = {
    "a8",
    "b8",
    "c8",
    "d8",
    "e8",
    "f8",
    "g8",
    "h8",
    "a7",
    "b7",
    "c7",
    "d7",
    "e7",
    "f7",
    "g7",
    "h7",
    "a6",
    "b6",
    "c6",
    "d6",
    "e6",
    "f6",
    "g6",
    "h6",
    "a5",
    "b5",
    "c5",
    "d5",
    "e5",
    "f5",
    "g5",
    "h5",
    "a4",
    "b4",
    "c4",
    "d4",
    "e4",
    "f4",
    "g4",
    "h4",
    "a3",
    "b3",
    "c3",
    "d3",
    "e3",
    "f3",
    "g3",
    "h3",
    "a2",
    "b2",
    "c2",
    "d2",
    "e2",
    "f2",
    "g2",
    "h2",
    "a1",
    "b1",
    "c1",
    "d1",
    "e1",
    "f1",
    "g1",
    "h1",
};
int material_score[12]={
    100,
    300,
    350,
    500,
    1000,
    10000,
    -100,
    -300,
    -350,
    -500,
    -1000,
    -10000
};
const int pawn_score[64] = 
{
    90,  90,  90,  90,  90,  90,  90,  90,
    30,  30,  30,  40,  40,  30,  30,  30,
    20,  20,  20,  30,  30,  30,  20,  20,
    10,  10,  10,  20,  20,  10,  10,  10,
     5,   5,  10,  20,  20,   5,   5,   5,
     0,   0,   0,   5,   5,   0,   0,   0,
     0,   0,   0, -10, -10,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0
};

// knight positional score
const int knight_score[64] = 
{
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,  10,  10,   0,   0,  -5,
    -5,   5,  20,  20,  20,  20,   5,  -5,
    -5,  10,  20,  30,  30,  20,  10,  -5,
    -5,  10,  20,  30,  30,  20,  10,  -5,
    -5,   5,  20,  10,  10,  20,   5,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5, -10,   0,   0,   0,   0, -10,  -5
};

// bishop positional score
const int bishop_score[64] = 
{
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   20,  0,  10,  10,   0,   20,  0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,  10,   0,   0,   0,   0,  10,   0,
     0,  30,   0,   0,   0,   0,  30,   0,
     0,   0, -10,   0,   0, -10,   0,   0

};

// rook positional score
const int rook_score[64] =
{
    50,  50,  50,  50,  50,  50,  50,  50,
    50,  50,  50,  50,  50,  50,  50,  50,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,   0,  20,  20,   0,   0,   0

};

// king positional score
const int king_score[64] = 
{
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   5,   5,   5,   5,   0,   0,
     0,   5,   5,  10,  10,   5,   5,   0,
     0,   5,  10,  20,  20,  10,   5,   0,
     0,   5,  10,  20,  20,  10,   5,   0,
     0,   0,   5,  10,  10,   5,   0,   0,
     0,   5,   5,  -5,  -5,   0,   5,   0,
     0,   0,   5,   0, -15,   0,  10,   0
};

// mirror positional score tables for opposite side
const int mirror_score[128] =
{
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8
};

char ascii_pieces[] = "PNBRQKpnbrqk";

// unicode pieces
const char *unicode_pieces[] = {
    "\u2659", "\u2658", "\u2657", "\u2656", "\u2655", "\u2654",
    "\u265F", "\u265E", "\u265D", "\u265C", "\u265B", "\u265A"};

std::unordered_map<char, int> char_pieces = {
    {'P', P},
    {'N', N},
    {'B', B},
    {'R', R},
    {'Q', Q},
    {'K', K},
    {'p', p},
    {'n', n},
    {'b', b},
    {'r', r},
    {'q', q},
    {'k', k}};

U64 bitboards[12];

U64 occupancies[3];

int side;

int enpassant = no_sqr;

int castle = 0;

U64 hash_key;

U64 repetition_table[1000];

int repetition_index;





int quit = 0;

int movestogo = 30;

int movetime = -1;

int timee = -1;

int inc = 0;

int starttime = 0;

int stoptimee = 0;

int timeset = 0;

int stopped = 0;

using namespace std;

// BIT MANIPULATIONS

// BIT MACROS
#define SETBIT(bb, sq) ((bb) |= (1ULL << (sq)))
#define GETBIT(bb, sq) ((bb) & (1ULL << (sq)))
#define CLEARBIT(bb, sq) ((bb) &= ~(1ULL << (sq)))
#define POPBIT(bb, sq) (GETBIT(bitboard, sq) ? CLEARBIT(bitboard, sq) : 0)
// Count bits within a bitboard

static inline int count_bits(U64 b)
{
    int r;
    for (r = 0; b; r++, b &= b - 1)
        ;
    return r;
}
// get least significant bit index
static inline int lsb(U64 b)
{
    return __builtin_ctzll(b);
}

void print_bitboard(U64 bitboard)
{
    cout << "\n";
    // loop over board ranks
    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            int square = rank * 8 + file;

            // print ranks
            if (!file)
            {
                cout << 8 - rank << "  ";
            }

            // print the bit at the square
            cout << (GETBIT(bitboard, square) ? 1 : 0) << " ";
        }
        cout << "\n";
    }
    cout << "\n   a b c d e f g h";
    cout << "\n\n";
    // print bitboard as unsigned decimal number
    cout << "   Bitboard: " << (bitboard) << "\n\n";
}

U64 soutOne(U64 b)
{
    return b >> 8;
}
U64 nortOne(U64 b)
{
    return b << 8;
}

void print_board()
{
    cout << "\n";
    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            int square = rank * 8 + file;
            if (!file)
            {
                cout << " " << 8 - rank << " ";
            }

            int piece = -1;
            for (int count = 0; count < 12; count++)
            {
                if (GETBIT(bitboards[count], square))
                {
                    piece = count;
                }
            }

#ifdef WIN64
            if (piece == -1)
                cout << ". ";
            else
                cout << ascii_pieces[piece] << " ";
#else
            if (piece == -1)
                cout << ". ";
            else
                cout << unicode_pieces[piece] << " ";
#endif
        }
        cout << "\n";
    }
    cout << "\n   a b c d e f g h\n\n";

    cout << "     SIDE: " << ((!side) ? "WHITE" : "BLACK") << "\n";

    cout << "     ENPASSANT: " << (enpassant == no_sqr ? "NO" : sq_to_str[enpassant]) << "\n";

    cout << "     CASTLE: " << ((castle & WK) ? 'K' : '-')
         << ((castle & WQ) ? 'Q' : '-')
         << ((castle & BK) ? 'k' : '-')
         << ((castle & BQ) ? 'q' : '-')
         << "\n\n";

    cout<<"     HASH KEY: "<<hex<<hash_key<<dec<<"\n\n";
}



    int get_time_ms()
    {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    }




int input_waiting() {
#ifndef WIN32
    fd_set readfds;
    struct timeval tv;
    FD_ZERO(&readfds);
    FD_SET(fileno(stdin), &readfds);
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    select(16, &readfds, nullptr, nullptr, &tv);
    return FD_ISSET(fileno(stdin), &readfds);
#else
    static int init = 0,pipe;
    static HANDLE inh;
    DWORD dw;

    if (!init) {
        init = 1;
        inh = GetStdHandle(STD_INPUT_HANDLE);
        pipe = !GetConsoleMode(inh, &dw);
        if (!pipe) {
            SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
            FlushConsoleInputBuffer(inh);
        }
    }

    if (pipe) {
        if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
        return dw;
    } else {
        GetNumberOfConsoleInputEvents(inh, &dw);
        return dw <= 1 ? 0 : dw;
    }
#endif
}



void read_input()
{
    // bytes to read holder
    int bytes;
    
    // GUI/user input
    char input[256] = "", *endc;

    // "listen" to STDIN
    if (input_waiting())
    {
        // tell engine to stop calculating
        stopped = 1;
        
        // loop to read bytes from STDIN
        do
        {
            // read bytes from STDIN
            bytes=read(fileno(stdin), input, 256);
        }
        
        // until bytes available
        while (bytes < 0);
        
        // searches for the first occurrence of '\n'
        endc = strchr(input,'\n');
        
        // if found new line set value at pointer to 0
        if (endc) *endc=0;
        
        // if input is available
        if (strlen(input) > 0)
        {
            // match UCI "quit" command
            if (!strncmp(input, "quit", 4))
            {
                // tell engine to terminate exacution    
                quit = 1;
            }

            // // match UCI "stop" command
            else if (!strncmp(input, "stop", 4))    {
                // tell engine to terminate exacution
                quit = 1;
            }
        }   
    }
}
static void communicate() {
	// if time is up break here
    if(timeset == 1 && get_time_ms() > stoptimee) {
		// tell engine to stop calculating
        cout<<"Times up! Stopping calculation\n";
		stopped = 1;
	}
	
    // read GUI input
	read_input();
}



// RANDOM NUMBER GENERATORS

unsigned int random_state = 1804289383;
unsigned int random_num_U32()
{
    unsigned int number = random_state;

    // XOR SHIFT
    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;

    // UPDATE RANDOM NUMBER STATE
    random_state = number;
    return number;
}

unsigned long long random_num_U64()
{
    // define 4 random numbers
    U64 n1, n2, n3, n4;
    
    // init random numbers slicing 16 bits from MS1B side
    n1 = (U64)(random_num_U32()) & 0xFFFF;
    n2 = (U64)(random_num_U32()) & 0xFFFF;
    n3 = (U64)(random_num_U32()) & 0xFFFF;
    n4 = (U64)(random_num_U32()) & 0xFFFF;
    
    // return random number
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

U64 generate_magic_number()
{
    return random_num_U64() & random_num_U64() & random_num_U64();
}

const U64 NOT_FILE_A = 18374403900871474942ULL;

const U64 NOT_FILE_H = 9187201950435737471ULL;

// NOT HG FILE
const U64 NOT_FILE_HG = 4557430888798830399ULL;

// NOT AB FILE
const U64 NOT_FILE_AB = 18229723555195321596ULL;

// RELEVENCY OCCUPANCY BIT COUNT
const int bishop_relevant_bits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6};

const int rook_relevant_bits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12};
U64 ROOK_MAGIC_NUMBER[64] = {
    0x8a80104000800020ULL,
    0x140002000100040ULL,
    0x2801880a0017001ULL,
    0x100081001000420ULL,
    0x200020010080420ULL,
    0x3001c0002010008ULL,
    0x8480008002000100ULL,
    0x2080088004402900ULL,
    0x800098204000ULL,
    0x2024401000200040ULL,
    0x100802000801000ULL,
    0x120800800801000ULL,
    0x208808088000400ULL,
    0x2802200800400ULL,
    0x2200800100020080ULL,
    0x801000060821100ULL,
    0x80044006422000ULL,
    0x100808020004000ULL,
    0x12108a0010204200ULL,
    0x140848010000802ULL,
    0x481828014002800ULL,
    0x8094004002004100ULL,
    0x4010040010010802ULL,
    0x20008806104ULL,
    0x100400080208000ULL,
    0x2040002120081000ULL,
    0x21200680100081ULL,
    0x20100080080080ULL,
    0x2000a00200410ULL,
    0x20080800400ULL,
    0x80088400100102ULL,
    0x80004600042881ULL,
    0x4040008040800020ULL,
    0x440003000200801ULL,
    0x4200011004500ULL,
    0x188020010100100ULL,
    0x14800401802800ULL,
    0x2080040080800200ULL,
    0x124080204001001ULL,
    0x200046502000484ULL,
    0x480400080088020ULL,
    0x1000422010034000ULL,
    0x30200100110040ULL,
    0x100021010009ULL,
    0x2002080100110004ULL,
    0x202008004008002ULL,
    0x20020004010100ULL,
    0x2048440040820001ULL,
    0x101002200408200ULL,
    0x40802000401080ULL,
    0x4008142004410100ULL,
    0x2060820c0120200ULL,
    0x1001004080100ULL,
    0x20c020080040080ULL,
    0x2935610830022400ULL,
    0x44440041009200ULL,
    0x280001040802101ULL,
    0x2100190040002085ULL,
    0x80c0084100102001ULL,
    0x4024081001000421ULL,
    0x20030a0244872ULL,
    0x12001008414402ULL,
    0x2006104900a0804ULL,
    0x1004081002402ULL};

U64 BISHOP_MAGIC_NUMBER[64] = {
    0x40040844404084ULL,
    0x2004208a004208ULL,
    0x10190041080202ULL,
    0x108060845042010ULL,
    0x581104180800210ULL,
    0x2112080446200010ULL,
    0x1080820820060210ULL,
    0x3c0808410220200ULL,
    0x4050404440404ULL,
    0x21001420088ULL,
    0x24d0080801082102ULL,
    0x1020a0a020400ULL,
    0x40308200402ULL,
    0x4011002100800ULL,
    0x401484104104005ULL,
    0x801010402020200ULL,
    0x400210c3880100ULL,
    0x404022024108200ULL,
    0x810018200204102ULL,
    0x4002801a02003ULL,
    0x85040820080400ULL,
    0x810102c808880400ULL,
    0xe900410884800ULL,
    0x8002020480840102ULL,
    0x220200865090201ULL,
    0x2010100a02021202ULL,
    0x152048408022401ULL,
    0x20080002081110ULL,
    0x4001001021004000ULL,
    0x800040400a011002ULL,
    0xe4004081011002ULL,
    0x1c004001012080ULL,
    0x8004200962a00220ULL,
    0x8422100208500202ULL,
    0x2000402200300c08ULL,
    0x8646020080080080ULL,
    0x80020a0200100808ULL,
    0x2010004880111000ULL,
    0x623000a080011400ULL,
    0x42008c0340209202ULL,
    0x209188240001000ULL,
    0x400408a884001800ULL,
    0x110400a6080400ULL,
    0x1840060a44020800ULL,
    0x90080104000041ULL,
    0x201011000808101ULL,
    0x1a2208080504f080ULL,
    0x8012020600211212ULL,
    0x500861011240000ULL,
    0x180806108200800ULL,
    0x4000020e01040044ULL,
    0x300000261044000aULL,
    0x802241102020002ULL,
    0x20906061210001ULL,
    0x5a84841004010310ULL,
    0x4010801011c04ULL,
    0xa010109502200ULL,
    0x4a02012000ULL,
    0x500201010098b028ULL,
    0x8040002811040900ULL,
    0x28000010020204ULL,
    0x6000020202d0240ULL,
    0x8918844842082200ULL,
    0x4010011029020020ULL};
// PAWN ATTACKS TABLE
U64 PAWN_ATTACKS[2][64];
// KNIGHT ATTACKS TABLE
U64 KNIGHT_ATTACKS[64];
// KING ATTACKS TABLE
U64 KING_ATTACKS[64];
// BISHOP ATTACKS TABLE
U64 BISHOP_ATTACKS[64][512];
// ROOK ATTACKS TABLE
U64 ROOK_ATTACKS[64][4096];

U64 BISHOP_MASKS[64];

U64 ROOK_MASKS[64];

// MASK ATTACKS
U64 mask_pawn_attacks(int side, int square)
{

    // results attack bitboard
    U64 attacks = 0ULL;

    // piece bitboard
    U64 bitboard = 0ULL;

    // set piece bitboard
    SETBIT(bitboard, square);

    // white pawns
    if (!side)
    {
        if (((bitboard >> 7)) & NOT_FILE_A)
            attacks |= (bitboard >> 7);
        if (((bitboard >> 9)) & NOT_FILE_H)
            attacks |= (bitboard >> 9);
    }
    // black pawns
    else
    {
        if ((bitboard << 7) & NOT_FILE_H)
            attacks |= (bitboard << 7);
        if ((bitboard << 9) & NOT_FILE_A)
            attacks |= (bitboard << 9);
    }

    return attacks;
}

U64 mask_knight_attacks(int square)
{
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    SETBIT(bitboard, square);

    if ((bitboard >> 17) & NOT_FILE_H)
        attacks |= (bitboard >> 17);
    if ((bitboard >> 15) & NOT_FILE_A)
        attacks |= (bitboard >> 15);
    if ((bitboard >> 10) & NOT_FILE_HG)
        attacks |= (bitboard >> 10);
    if ((bitboard >> 6) & NOT_FILE_AB)
        attacks |= (bitboard >> 6);

    if ((bitboard << 17) & NOT_FILE_A)
        attacks |= (bitboard << 17);
    if ((bitboard << 15) & NOT_FILE_H)
        attacks |= (bitboard << 15);
    if ((bitboard << 10) & NOT_FILE_AB)
        attacks |= (bitboard << 10);
    if ((bitboard << 6) & NOT_FILE_HG)
        attacks |= (bitboard << 6);

    return attacks;
}

U64 mask_king_attacks(int square)
{
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    SETBIT(bitboard, square);

    if ((bitboard >> 8))
        attacks |= (bitboard >> 8);
    if ((bitboard >> 9) & NOT_FILE_H)
        attacks |= (bitboard >> 9);
    if ((bitboard >> 7) & NOT_FILE_A)
        attacks |= (bitboard >> 7);
    if ((bitboard >> 1) & NOT_FILE_H)
        attacks |= (bitboard >> 1);

    if ((bitboard << 8))
        attacks |= (bitboard << 8);
    if ((bitboard << 9) & NOT_FILE_A)
        attacks |= (bitboard << 9);
    if ((bitboard << 7) & NOT_FILE_H)
        attacks |= (bitboard << 7);
    if ((bitboard << 1) & NOT_FILE_A)
        attacks |= (bitboard << 1);

    return attacks;
}

U64 mask_bishop_attacks(int square)
{
    U64 attacks = 0ULL;
    // INITIALIZE RANKS AND FILES
    int r, f;
    // INITIALIZE TARGETS
    int tr = square / 8;
    int tf = square % 8;

    for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++)
    {
        SETBIT(attacks, r * 8 + f);
    }
    for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++)
    {
        SETBIT(attacks, r * 8 + f);
    }
    for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--)
    {
        SETBIT(attacks, r * 8 + f);
    }
    for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--)
    {
        SETBIT(attacks, r * 8 + f);
    }
    return attacks;
}

U64 mask_rook_attacks(int square)
{
    U64 attacks = 0ULL;
    // INITIALIZE RANKS AND FILES
    int r, f;
    // INITIALIZE TARGETS
    int tr = square / 8;
    int tf = square % 8;

    for (r = tr + 1; r <= 6; r++)
    {
        SETBIT(attacks, r * 8 + tf);
    }
    for (r = tr - 1; r >= 1; r--)
    {
        SETBIT(attacks, r * 8 + tf);
    }
    for (f = tf + 1; f <= 6; f++)
    {
        SETBIT(attacks, tr * 8 + f);
    }
    for (f = tf - 1; f >= 1; f--)
    {
        SETBIT(attacks, tr * 8 + f);
    }

    return attacks;
}
U64 bishop_attacks_fly(int square, U64 block)
{
    U64 attacks = 0ULL;
    // INITIALIZE RANKS AND FILES
    int r, f;
    // INITIALIZE TARGETS
    int tr = square / 8;
    int tf = square % 8;

    for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++)
    {
        SETBIT(attacks, r * 8 + f);
        if (1ULL << (r * 8 + f) & block)
            break;
    }
    for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++)
    {
        SETBIT(attacks, r * 8 + f);
        if (1ULL << (r * 8 + f) & block)
            break;
    }
    for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--)
    {
        SETBIT(attacks, r * 8 + f);
        if (1ULL << (r * 8 + f) & block)
            break;
    }
    for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--)
    {
        SETBIT(attacks, r * 8 + f);
        if (1ULL << (r * 8 + f) & block)
            break;
    }
    return attacks;
}
U64 rook_attacks_fly(int square, U64 block)
{
    U64 attacks = 0ULL;
    // INITIALIZE RANKS AND FILES
    int r, f;
    // INITIALIZE TARGETS
    int tr = square / 8;
    int tf = square % 8;

    for (r = tr + 1; r <= 7; r++)
    {
        SETBIT(attacks, r * 8 + tf);
        if (1ULL << (r * 8 + tf) & block)
            break;
    }
    for (r = tr - 1; r >= 0; r--)
    {
        SETBIT(attacks, r * 8 + tf);
        if (1ULL << (r * 8 + tf) & block)
            break;
    }
    for (f = tf + 1; f <= 7; f++)
    {
        SETBIT(attacks, tr * 8 + f);
        if (1ULL << (tr * 8 + f) & block)
            break;
    }
    for (f = tf - 1; f >= 0; f--)
    {
        SETBIT(attacks, tr * 8 + f);
        if (1ULL << (tr * 8 + f) & block)
            break;
    }

    return attacks;
}
U64 fill_occupancy(int index, int bits_in_mask, U64 attack_mask)
{
    // Ocuppied squares
    U64 occupancy = 0ULL;
    for (int count = 0; count < bits_in_mask; count++)
    {
        int square = lsb(attack_mask);
        CLEARBIT(attack_mask, square);
        if (index & (1 << count))
        {
            SETBIT(occupancy, square);
        }
    }

    return occupancy;
}

static inline U64 get_bishop_attacks(int square, U64 occupancy)
{
    occupancy &= BISHOP_MASKS[square];
    occupancy *= BISHOP_MAGIC_NUMBER[square];
    occupancy >>= 64 - bishop_relevant_bits[square];

    return BISHOP_ATTACKS[square][occupancy];
}

static inline U64 get_rook_attacks(int square, U64 occupancy)
{
    occupancy &= ROOK_MASKS[square];
    occupancy *= ROOK_MAGIC_NUMBER[square];
    occupancy >>= 64 - rook_relevant_bits[square];

    return ROOK_ATTACKS[square][occupancy];
}

static inline U64 get_queen_attacks(int square, U64 occupancy)
{
    U64 result = 0ULL;
    U64 bishopOccupancy = occupancy;
    U64 rookOccupancy = occupancy;

    bishopOccupancy &= BISHOP_MASKS[square];
    bishopOccupancy *= BISHOP_MAGIC_NUMBER[square];
    bishopOccupancy >>= 64 - bishop_relevant_bits[square];

    result |= BISHOP_ATTACKS[square][bishopOccupancy];

    rookOccupancy &= ROOK_MASKS[square];
    rookOccupancy *= ROOK_MAGIC_NUMBER[square];
    rookOccupancy >>= 64 - rook_relevant_bits[square];

    result |= ROOK_ATTACKS[square][rookOccupancy];

    return result;
}

static inline int is_square_attacked(int square, int side)
{
    if ((side == WHITE) && (PAWN_ATTACKS[BLACK][square] & bitboards[P]))
        return 1;

    if ((side == BLACK) && (PAWN_ATTACKS[WHITE][square] & bitboards[p]))
        return 1;

    if (KNIGHT_ATTACKS[square] & (side == WHITE ? bitboards[N] : bitboards[n]))
        return 1;

    if (KING_ATTACKS[square] & (side == WHITE ? bitboards[K] : bitboards[k]))
        return 1;

    if (get_bishop_attacks(square, occupancies[BOTH]) & (side == WHITE ? bitboards[B] : bitboards[b]))
        return 1;

    if (get_rook_attacks(square, occupancies[BOTH]) & (side == WHITE ? bitboards[R] : bitboards[r]))
        return 1;

    if (get_queen_attacks(square, occupancies[BOTH]) & (side == WHITE ? bitboards[Q] : bitboards[q]))
        return 1;

    return 0;
}

// INITIALIZE LEAPER ATTACKS
void init_leaper_attacks()
{
    // KNIGHT ATTACKS
    for (int square = 0; square < 64; square++)
    {
        // INITIALIZE KNIGHT ATTACKS
        KNIGHT_ATTACKS[square] = mask_knight_attacks(square);
        // INITIALIZE PAWN ATTACKS
        PAWN_ATTACKS[WHITE][square] = mask_pawn_attacks(WHITE, square);
        PAWN_ATTACKS[BLACK][square] = mask_pawn_attacks(BLACK, square);

        // INITIALIZE KING ATTACKS
        KING_ATTACKS[square] = mask_king_attacks(square);
    }
}

void init_slider_attacks(int bishop)
{

    for (int square = 0; square < 64; square++)
    {
        BISHOP_MASKS[square] = mask_bishop_attacks(square);
        ROOK_MASKS[square] = mask_rook_attacks(square);

        U64 attack_mask = (bishop) ? BISHOP_MASKS[square] : ROOK_MASKS[square];

        int relevant_bits_count = count_bits(attack_mask);

        int occupancy_indicies = (1 << relevant_bits_count);

        for (int index = 0; index < occupancy_indicies; index++)
        {
            if (bishop)
            {
                U64 occupancy = fill_occupancy(index, relevant_bits_count, attack_mask);

                uint64_t magic_index = static_cast<uint64_t>((occupancy * BISHOP_MAGIC_NUMBER[square]) >> (64 - bishop_relevant_bits[square]));

                // INITIALIZE BISHOP ATTACKS
                BISHOP_ATTACKS[square][magic_index] = bishop_attacks_fly(square, occupancy);
            }
            else
            {
                U64 occupancy = fill_occupancy(index, relevant_bits_count, attack_mask);

                int magic_index = (occupancy * ROOK_MAGIC_NUMBER[square]) >> (64 - rook_relevant_bits[square]);

                // INITIALIZE BISHOP ATTACKS
                ROOK_ATTACKS[square][magic_index] = rook_attacks_fly(square, occupancy);
            }
        }
    }
}

int transform(U64 b, U64 magic, int bits)
{
#if defined(USE_32_BIT_MULTIPLICATIONS)
    return (unsigned)((int)b * (int)magic ^ (int)(b >> 32) * (int)(magic >> 32)) >> (32 - bits);
#else
    return (int)((b * magic) >> (64 - bits));
#endif
}

U64 find_magic_number(int square, int relevant_bits, int bishop)
{

    U64 occupancies[4096];
    U64 used_attacks[4096];
    U64 attacks[4096];

    U64 attack_mask = bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);

    int occupancy_indicies = 1 << relevant_bits;

    // LOOP OVER ALL OCCUPANCY VARIATIONS
    for (int index = 0; index < occupancy_indicies; index++)
    {
        // FILL OCCUPANCY
        occupancies[index] = fill_occupancy(index, relevant_bits, attack_mask);
        attacks[index] = bishop ? bishop_attacks_fly(square, occupancies[index]) : rook_attacks_fly(square, occupancies[index]);
    }

    for (int random_count = 0; random_count < 100000000; random_count++)
    {

        U64 magic_number = generate_magic_number();

        if (count_bits((attack_mask * magic_number) & 0xFF00000000000000ULL) < 6)
        {
            continue;
        }

        for (int i = 0; i < 4096; i++)
        {
            used_attacks[i] = 0ULL;
        }

        int index, fail;

        for (index = 0, fail = 0; !fail && index < occupancy_indicies; index++)
        {
            int magic_index = (int)((occupancies[index] * magic_number) >> (64 - relevant_bits));
            if (used_attacks[magic_index] == 0ULL)
                used_attacks[magic_index] = attacks[index];

            else if (used_attacks[magic_index] != attacks[index])
                fail = 1;
        }
        if (!fail)
        {
            return magic_number;
        }
    }

    cout << ("MAGIC NUMBER NOT FOUND\n");
    return 0ULL;
}

void init_magic_numbers()
{
    for (int square = 0; square < 64; square++)
    {
        ROOK_MAGIC_NUMBER[square] = find_magic_number(square, rook_relevant_bits[square], ROOK);
        cout << ROOK_MAGIC_NUMBER[square] << "\n";
    }
    for (int square = 0; square < 64; square++)
    {
        BISHOP_MAGIC_NUMBER[square] = find_magic_number(square, bishop_relevant_bits[square], BISHOP);
    }
}
U64 generate_hash_key()
{
    U64 final_key = 0ULL;
    U64 bitboard;

    for(int piece=0;piece<12;piece++)
    {
        bitboard = bitboards[piece];
        while(bitboard)
        {
            int square = lsb(bitboard);
            final_key ^= piece_keys[piece][square];
            CLEARBIT(bitboard,square);
        }
    }
    if(enpassant!=no_sqr)
    {
        final_key ^= enpassant_keys[enpassant];
    }
    final_key ^= castle_keys[castle];
    if(side==BLACK) 
    {
        final_key ^= side_key;
    }
    return final_key;
}

unordered_map<int, char> promoted_pieces = {{Q, 'q'}, {R, 'r'}, {B, 'b'}, {N, 'n'}, {q, 'q'}, {r, 'r'}, {b, 'b'}, {n, 'n'}};
static inline void add_move(int move, moves *move_list)
{
    move_list->moves[move_list->move_count] = move;
    move_list->move_count++;
}
void print_move(int move)
{
    int source = FROMSQ(move);
    int target = TOSQ(move);
    int piece = GET_PIECE(move);
    int promoted = GET_PROMOTED(move);
    int capture = (GET_CAPTURE(move)) ? 1 : 0;
    int double_push = (GET_DOUBLE(move)) ? 1 : 0;
    int enpass = (GET_ENPASSANT(move)) ? 1 : 0;
    int castling = (GET_CASTLING(move)) ? 1 : 0;

    cout << sq_to_str[source] << sq_to_str[target];
    if (promoted)
    {
        cout << promoted_pieces[promoted];
    }
}
void print_move_list(moves *move_list)
{
    if (!move_list->move_count)
        return;
    cout << "Move List\n";
    for (int index = 0; index < move_list->move_count; index++)
    {
        cout << index + 1 << ". ";
        print_move(move_list->moves[index]);
    }
}

static inline void generate_moves(moves *move_list)
{
    move_list->move_count = 0;
    int source_square, target_square;

    U64 bitboard, attacks;

    for (int piece = P; piece <= k; piece++)
    {
        bitboard = bitboards[piece];

        // GENERATE WHITE PAWNS AND WHITE CASTLING MOVES
        if (side == WHITE)
        {
            if (piece == P)
            {
                while (bitboard)
                {
                    source_square = lsb(bitboard);

                    target_square = source_square - 8;

                    // QUIET MOVE
                    if (!(target_square < a8) && !(GETBIT(occupancies[BOTH], target_square)))
                    {
                        // PAWN PROMOTION
                        if (source_square >= a7 && source_square <= h7)
                        {

                            add_move(ENCODE_MOVE(source_square, target_square, piece, Q, 0, 0, 0, 0), move_list);
                            add_move(ENCODE_MOVE(source_square, target_square, piece, R, 0, 0, 0, 0), move_list);
                            add_move(ENCODE_MOVE(source_square, target_square, piece, B, 0, 0, 0, 0), move_list);
                            add_move(ENCODE_MOVE(source_square, target_square, piece, N, 0, 0, 0, 0), move_list);
                        }
                        else
                        {
                            // ONE SQUARE
                            add_move(ENCODE_MOVE(source_square, target_square, piece, 0, 0, 0, 0, 0), move_list);

                            if ((source_square >= a2 && source_square <= h2) && !(GETBIT(occupancies[BOTH], target_square - 8)))
                            {
                                add_move(ENCODE_MOVE(source_square, target_square - 8, piece, 0, 0, 1, 0, 0), move_list);
                            }
                        }
                    }

                    attacks = PAWN_ATTACKS[side][source_square] & occupancies[BLACK];

                    while (attacks)
                    {
                        // INIT TARGET SQUARE
                        target_square = lsb(attacks);
                        // POP BIT
                        if (source_square >= a7 && source_square <= h7)
                        {
                            add_move(ENCODE_MOVE(source_square, target_square, piece, Q, 1, 0, 0, 0), move_list);
                            add_move(ENCODE_MOVE(source_square, target_square, piece, R, 1, 0, 0, 0), move_list);
                            add_move(ENCODE_MOVE(source_square, target_square, piece, B, 1, 0, 0, 0), move_list);
                            add_move(ENCODE_MOVE(source_square, target_square, piece, N, 1, 0, 0, 0), move_list);
                        }
                        else
                        {
                            // ONE SQUARE
                            add_move(ENCODE_MOVE(source_square, target_square, piece, 0, 1, 0, 0, 0), move_list);
                        }

                        CLEARBIT(attacks, target_square);
                    }
                    if (enpassant != no_sqr)
                    {
                        U64 enpassant_attacks = PAWN_ATTACKS[side][source_square] & (1ULL << enpassant);

                        if (enpassant_attacks)
                        {
                            int target_enpassant = lsb(enpassant_attacks);
                            add_move(ENCODE_MOVE(source_square, target_enpassant, piece, 0, 1, 0, 1, 0), move_list);
                        }
                    }

                    // POP BIT
                    CLEARBIT(bitboard, source_square);
                }
            }

            // CASTLING MOVES
            if (piece == K)
            {
                if (castle & WK)
                {
                    if (!(GETBIT(occupancies[BOTH], f1)) && !(GETBIT(occupancies[BOTH], g1)))
                    {
                        if (!is_square_attacked(e1, BLACK) && !is_square_attacked(f1, BLACK))
                        {
                            add_move(ENCODE_MOVE(e1, g1, piece, 0, 0, 0, 0, 1), move_list);
                        }
                    }
                }
                if (castle & WQ)
                {
                    if (!(occupancies[BOTH] & (1ULL << d1)) && !(occupancies[BOTH] & (1ULL << c1)) && !(occupancies[BOTH] & (1ULL << b1)))
                    {
                        if (!is_square_attacked(e1, BLACK) && !is_square_attacked(d1, BLACK))
                        {
                            add_move(ENCODE_MOVE(e1, c1, piece, 0, 0, 0, 0, 1), move_list);
                        }
                    }
                }
            }
        }
        else
        {
            if (piece == p)
            {
                while (bitboard)
                {
                    source_square = lsb(bitboard);

                    target_square = source_square + 8;

                    // QUIET MOVE
                    if (!(target_square > h1) && !(GETBIT(occupancies[BOTH], target_square)))
                    {
                        // PAWN PROMOTION
                        if (source_square >= a2 && source_square <= h2)
                        {
                            add_move(ENCODE_MOVE(source_square, target_square, piece, q, 0, 0, 0, 0), move_list);
                            add_move(ENCODE_MOVE(source_square, target_square, piece, r, 0, 0, 0, 0), move_list);
                            add_move(ENCODE_MOVE(source_square, target_square, piece, b, 0, 0, 0, 0), move_list);
                            add_move(ENCODE_MOVE(source_square, target_square, piece, n, 0, 0, 0, 0), move_list);
                        }
                        else
                        {
                            // ONE SQUARE
                            add_move(ENCODE_MOVE(source_square, target_square, piece, 0, 0, 0, 0, 0), move_list);

                            if ((source_square >= a7 && source_square <= h7) && !(GETBIT(occupancies[BOTH], target_square + 8)))
                            {
                                add_move(ENCODE_MOVE(source_square, target_square + 8, piece, 0, 0, 1, 0, 0), move_list);
                            }
                        }
                    }
                    attacks = PAWN_ATTACKS[side][source_square] & occupancies[WHITE];

                    while (attacks)
                    {
                        // INIT TARGET SQUARE
                        target_square = lsb(attacks);
                        // POP BIT
                        if (source_square >= a2 && source_square <= h2)
                        {

                            add_move(ENCODE_MOVE(source_square, target_square, piece, q, 1, 0, 0, 0), move_list);
                            add_move(ENCODE_MOVE(source_square, target_square, piece, r, 1, 0, 0, 0), move_list);
                            add_move(ENCODE_MOVE(source_square, target_square, piece, b, 1, 0, 0, 0), move_list);
                            add_move(ENCODE_MOVE(source_square, target_square, piece, n, 1, 0, 0, 0), move_list);
                        }
                        else
                        {
                            // ONE SQUARE
                            add_move(ENCODE_MOVE(source_square, target_square, piece, 0, 1, 0, 0, 0), move_list);
                        }

                        CLEARBIT(attacks, target_square);
                    }
                    if (enpassant != no_sqr)
                    {
                        U64 enpassant_attacks = PAWN_ATTACKS[side][source_square] & (1ULL << enpassant);

                        if (enpassant_attacks)
                        {
                            int target_enpassant = lsb(enpassant_attacks);
                            add_move(ENCODE_MOVE(source_square, target_enpassant, piece, 0, 1, 0, 1, 0), move_list);
                        }
                    }
                    // POP BIT
                    CLEARBIT(bitboard, source_square);
                }
            }
            // CASTLING MOVES
            if (piece == k)
            {
                if (castle & BK)
                {
                    if (!(GETBIT(occupancies[BOTH], f8)) && !(GETBIT(occupancies[BOTH], g8)))
                    {
                        if (!is_square_attacked(e8, WHITE) && !is_square_attacked(f8, WHITE))
                        {
                            add_move(ENCODE_MOVE(e8, g8, piece, 0, 0, 0, 0, 1), move_list);
                        }
                    }
                }
                if (castle & BQ)
                {
                    if (!(occupancies[BOTH] & (1ULL << d8)) && !(occupancies[BOTH] & (1ULL << c8)) && !(occupancies[BOTH] & (1ULL << b8)))
                    {
                        if (!is_square_attacked(e8, WHITE) && !is_square_attacked(d8, WHITE))
                        {
                            add_move(ENCODE_MOVE(e8, c8, piece, 0, 0, 0, 0, 1), move_list);
                        }
                    }
                }
            }
        }

        // GENERATE KNIGHT MOVES
        if ((side == WHITE) ? piece == N : piece == n)
        {
            while (bitboard)
            {
                source_square = lsb(bitboard);

                attacks = KNIGHT_ATTACKS[source_square] & ((side == WHITE) ? ~occupancies[WHITE] : ~occupancies[BLACK]);

                while (attacks)
                {
                    target_square = lsb(attacks);
                    if (!GETBIT((side == WHITE ? occupancies[BLACK] : occupancies[WHITE]), target_square))
                    {
                        add_move(ENCODE_MOVE(source_square, target_square, piece, 0, 0, 0, 0, 0), move_list);
                    }
                    else
                    {
                        add_move(ENCODE_MOVE(source_square, target_square, piece, 0, 1, 0, 0, 0), move_list);
                    }

                    CLEARBIT(attacks, target_square);
                }
                CLEARBIT(bitboard, source_square);
            }
        }

        // GENERATE BISHOP MOVES
        if ((side == WHITE) ? piece == B : piece == b)
        {
            while (bitboard)
            {
                source_square = lsb(bitboard);

                attacks = get_bishop_attacks(source_square, occupancies[BOTH]) & (side == WHITE ? ~occupancies[WHITE] : ~occupancies[BLACK]);

                while (attacks)
                {
                    target_square = lsb(attacks);
                    if (!GETBIT((side == WHITE ? occupancies[BLACK] : occupancies[WHITE]), target_square))
                    {
                        add_move(ENCODE_MOVE(source_square, target_square, piece, 0, 0, 0, 0, 0), move_list);
                    }
                    else
                    {
                        add_move(ENCODE_MOVE(source_square, target_square, piece, 0, 1, 0, 0, 0), move_list);
                    }
                    CLEARBIT(attacks, target_square);
                }
                CLEARBIT(bitboard, source_square);
            }
        }

        // GENERATE ROOK MOVES
        if ((side == WHITE) ? piece == R : piece == r)
        {
            while (bitboard)
            {
                source_square = lsb(bitboard);

                attacks = get_rook_attacks(source_square, occupancies[BOTH]) & (side == WHITE ? ~occupancies[WHITE] : ~occupancies[BLACK]);

                while (attacks)
                {
                    target_square = lsb(attacks);
                    if (!GETBIT(side == WHITE ? occupancies[BLACK] : occupancies[WHITE], target_square))
                    {
                        add_move(ENCODE_MOVE(source_square, target_square, piece, 0, 0, 0, 0, 0), move_list);
                    }
                    else
                    {
                        add_move(ENCODE_MOVE(source_square, target_square, piece, 0, 1, 0, 0, 0), move_list);
                    }
                    CLEARBIT(attacks, target_square);
                }
                CLEARBIT(bitboard, source_square);
            }
        }

        // GENERATE QUEEN MOVES
        if ((side == WHITE) ? piece == Q : piece == q)
        {
            while (bitboard)
            {
                source_square = lsb(bitboard);

                attacks = get_queen_attacks(source_square, occupancies[BOTH]) & (side == WHITE ? ~occupancies[WHITE] : ~occupancies[BLACK]);

                while (attacks)
                {
                    target_square = lsb(attacks);
                    if (!GETBIT(side == WHITE ? occupancies[BLACK] : occupancies[WHITE], target_square))
                    {
                        add_move(ENCODE_MOVE(source_square, target_square, piece, 0, 0, 0, 0, 0), move_list);
                    }
                    else
                    {
                        add_move(ENCODE_MOVE(source_square, target_square, piece, 0, 1, 0, 0, 0), move_list);
                    }
                    CLEARBIT(attacks, target_square);
                }
                CLEARBIT(bitboard, source_square);
            }
        }

        // GENERATE KING MOVES
        if ((side == WHITE) ? piece == K : piece == k)
        {
            while (bitboard)
            {
                source_square = lsb(bitboard);

                attacks = KING_ATTACKS[source_square] & (side == WHITE ? ~occupancies[WHITE] : ~occupancies[BLACK]);

                while (attacks)
                {
                    target_square = lsb(attacks);
                    if (!GETBIT(side == WHITE ? occupancies[BLACK] : occupancies[WHITE], target_square))
                    {
                        add_move(ENCODE_MOVE(source_square, target_square, piece, 0, 0, 0, 0, 0), move_list);
                    }
                    else
                    {
                        add_move(ENCODE_MOVE(source_square, target_square, piece, 0, 1, 0, 0, 0), move_list);
                    }
                    CLEARBIT(attacks, target_square);
                }
                CLEARBIT(bitboard, source_square);
            }
        }
    }
}

static inline int make_move(int move, int move_flag)
{
    if (move_flag == all_moves)
    {
        // PRESERVE BOARD STATE
        COPY_BOARD();
        int source = FROMSQ(move);
        int target = TOSQ(move);
        int piece = GET_PIECE(move);
        int promoted = GET_PROMOTED(move);
        int capture = GET_CAPTURE(move);
        int double_push = GET_DOUBLE(move);
        int enpass = GET_ENPASSANT(move);
        int castling = GET_CASTLING(move);

        CLEARBIT(bitboards[piece], source);
        SETBIT(bitboards[piece], target);

        hash_key ^= piece_keys[piece][source];
        hash_key ^= piece_keys[piece][target];


        if (capture)
        {
            int start_piece, end_piece;
            if (side == WHITE)
            {
                start_piece = p;
                end_piece = k;
            }

            else
            {
                start_piece = P;
                end_piece = K;
            }

            for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++)
            {
                if (GETBIT(bitboards[bb_piece], target))
                {
                    CLEARBIT(bitboards[bb_piece], target);
                    hash_key ^= piece_keys[bb_piece][target];
                    break;
                }
            }
        }
        if (promoted)
        {
            CLEARBIT(bitboards[(side == WHITE) ? P : p], target);
            hash_key ^= piece_keys[(side == WHITE) ? P : p][target];
            SETBIT(bitboards[promoted], target);
            hash_key ^= piece_keys[promoted][target];
        }
        if (enpass)
        {
            if (side == WHITE)
            {
                CLEARBIT(bitboards[p], target + 8);
                hash_key ^= piece_keys[p][target + 8];
            }
            else
            {
                CLEARBIT(bitboards[P], target - 8);
                hash_key ^= piece_keys[P][target - 8];
            }
        }

        if(enpassant!=no_sqr)
        {
            hash_key ^= enpassant_keys[enpassant];
        }

        enpassant = no_sqr;

        if (double_push)
        {
            if(side == WHITE)
            {
                enpassant = target + 8;
                hash_key ^= enpassant_keys[target + 8];
            }
            else
            {
                enpassant = target - 8;
                hash_key ^= enpassant_keys[target - 8];
            }
        }

        if (castling)
        {
            switch (target)
            {
            case g1:
                CLEARBIT(bitboards[R], h1);
                SETBIT(bitboards[R], f1);
                hash_key ^= piece_keys[R][h1];
                hash_key ^= piece_keys[R][f1];
                break;
            case c1:
                CLEARBIT(bitboards[R], a1);
                SETBIT(bitboards[R], d1);
                hash_key ^= piece_keys[R][a1];
                hash_key ^= piece_keys[R][d1];
                break;
            case g8:
                CLEARBIT(bitboards[r], h8);
                SETBIT(bitboards[r], f8);
                hash_key ^= piece_keys[r][h8];
                hash_key ^= piece_keys[r][f8];
                break;
            case c8:
                CLEARBIT(bitboards[r], a8);
                SETBIT(bitboards[r], d8);
                hash_key ^= piece_keys[r][a8];
                hash_key ^= piece_keys[r][d8];
                break;
            }
        }
        hash_key ^= castle_keys[castle];
        castle &= castling_rights[source] & castling_rights[target];
        hash_key ^= castle_keys[castle];
        memset(occupancies, 0ULL, 24);

        for (int bb_piece = P; bb_piece <= K; bb_piece++)
        {
            occupancies[WHITE] |= bitboards[bb_piece];
        }
        for (int bb_piece = p; bb_piece <= k; bb_piece++)
        {
            occupancies[BLACK] |= bitboards[bb_piece];
        }

        occupancies[BOTH] = occupancies[WHITE] | occupancies[BLACK];

        side ^= 1;
        hash_key ^= side_key;

        if (is_square_attacked((side == WHITE) ? lsb(bitboards[k]) : lsb(bitboards[K]), side))
        {
            RESET_BOARD();
            return 0;
        }
        else
            return 1;
    }
    else
    {
        if (GET_CAPTURE(move))
        {
            make_move(move, all_moves);
        }
        else
            return 0;
    }
}
void init_random_keys()
{
    random_state = 1804289383;
    for(int piece=0;piece<12;piece++)
    {
        for(int square=0;square<64;square++)
        {
            piece_keys[piece][square] = random_num_U64();
        }
    }
    for(int square=0;square<64;square++)
    {
        enpassant_keys[square] = random_num_U64();
    }
    for(int i=0;i<16;i++)
    {
        castle_keys[i] = random_num_U64();
    }
    side_key = random_num_U64();
}





void print_attacked_squares(int side)
{
    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            int square = rank * 8 + file;
            if (!file)
            {
                cout << " " << 8 - rank << "  ";
            }
            if (is_square_attacked(square, side))
                cout << " 1";
            else
                cout << " 0";
        }
        cout << "\n";
    }
    cout << "\n     a b c d e f g h\n\n";
}

long nodes;

static inline void perft_driver(int depth)
{
    if (depth == 0)
    {
        nodes++;
        return;
    }

    moves move_list[1];
    generate_moves(move_list);

    for (int move = 0; move < move_list->move_count; move++)
    {

        COPY_BOARD();

        if (!make_move(move_list->moves[move], all_moves))
            continue;

        perft_driver(depth - 1);
        RESET_BOARD();
    }
}
void perft_test(int depth)
{
    if (depth == 0)
    {
        nodes++;
        return;
    }

    moves move_list[1];
    generate_moves(move_list);
    auto start_time = time_MS();

    for (int move = 0; move < move_list->move_count; move++)
    {

        COPY_BOARD();

        if (!make_move(move_list->moves[move], all_moves))
            continue;

        auto cummulative_nodes = nodes;

        perft_driver(depth - 1);

        auto old_nodes = nodes - cummulative_nodes;

        RESET_BOARD();
        cout << "Move: " << sq_to_str[FROMSQ(move_list->moves[move])] << sq_to_str[TOSQ(move_list->moves[move])] << promoted_pieces[GET_PROMOTED(move_list->moves[move])] << " Nodes: " << old_nodes << "\n";
    }
    auto stop_time = time_MS();
    cout << "Depth: " << depth << "\n";
    cout << "Nodes: " << nodes << "\n";
    cout << "time: " << chrono::duration_cast<chrono::milliseconds>(stop_time - start_time).count() << "ms\n";
}

int parse_move(char *move_string)
{
    moves move_list[1];

    generate_moves(move_list);
    int source_sq = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;
    cout << "Source: " << sq_to_str[source_sq] << "\n";
    int target_sq = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;
    cout << "Target: " << sq_to_str[target_sq] << "\n";

    for (int move_count = 0; move_count < move_list->move_count; move_count++)
    {
        int move = move_list->moves[move_count];
        if (FROMSQ(move) == source_sq && TOSQ(move) == target_sq)
        {
            int promoted_piece = GET_PROMOTED(move);
            if (promoted_piece)
            {

                if ((promoted_piece == Q || promoted_piece == q) && move_string[4] == 'q')
                {
                    return move;
                }
                else if ((promoted_piece == R || promoted_piece == r) && move_string[4] == 'r')
                {
                    return move;
                }
                else if ((promoted_piece == B || promoted_piece == b) && move_string[4] == 'b')
                {
                    return move;
                }
                else if ((promoted_piece == N || promoted_piece == n) && move_string[4] == 'n')
                {
                    return move;
                }

                continue;
            }

            return move;
        }
    }
    return 0;
}

U64 set_file_rank_mask(int file_number, int rank_number)
{
    U64 mask = 0ULL;

    for(int rank=0;rank<8;rank++)
    {
        for(int file=0;file<8;file++)
        {
            int square = rank*8+file;
            if(file==file_number || rank==rank_number)
            {
                mask |= SETBIT(mask,square);
            }
        }
    }


    return mask;
}

void init_evaluation_masks()
{
    for(int square=0;square<64;square++)
    {
        file_masks[square] = set_file_rank_mask(square%8,-1);
        rank_masks[square] = set_file_rank_mask(-1,square/8);
        isolated_masks[square] |= set_file_rank_mask((square%8 - 1),-1) | set_file_rank_mask((square%8 + 1),-1) | set_file_rank_mask(square%8,-1);
        
        // print_bitboard(white_passed_masks[square]);
    }
    for(int rank=0; rank<8;rank++)
    {
        for(int file=0;file<8;file++)
        {
            int square = rank*8+file;
            white_passed_masks[square] = set_file_rank_mask(file,-1) | set_file_rank_mask((file-1),-1) | set_file_rank_mask((file+1),-1);
            black_passed_masks[square] = set_file_rank_mask(file,-1) | set_file_rank_mask((file-1),-1) | set_file_rank_mask((file+1),-1);
            for(int i=0;i<(8-rank);i++)
            {
                white_passed_masks[square] &= ~rank_masks[(7-i)*8+file];
            }
            for(int i=0;i<rank + 1;i++)
            {
                black_passed_masks[square] &= ~rank_masks[i*8+file];
            }
        }
    }

    
}
const int double_pawn_penalty = -10;
const int isolated_pawn_penalty = -10;
const int passed_pawn_bonus[8] = { 0, 10, 30, 50, 75, 100, 150, 200 }; 
const int semi_open_file_bonus = 10;
const int open_file_bonus = 15;
const int bishop_pair_bonus = 30;
const int king_safety_bonus = 5;

static inline int evaluate()
{
    int score=0;

    U64 bitboard;

    int piece, square;

    int double_pawns = 0;

    for(int bb_piece =P;bb_piece<=k;bb_piece++)
    {
        bitboard = bitboards[bb_piece];

        while(bitboard)
        {
            piece = bb_piece;
            square = lsb(bitboard);
            score+=material_score[piece];
            switch(piece)
            {
                case P: 
                    score+=pawn_score[square];
                    double_pawns = count_bits(bitboards[P] & file_masks[square]);
                    if(double_pawns>1)
                    {
                        score+=double_pawns*double_pawn_penalty;
                    }
                    if((bitboards[P] & isolated_masks[square])==0)
                    {
                        score+=isolated_pawn_penalty;
                    }
                    if((white_passed_masks[square] & bitboards[p])==0)
                    {
                        score+=passed_pawn_bonus[square%8];
                    }
                    break;
                case N: score+=knight_score[square];break;
                case B: 
                    score+=bishop_score[square];
                    //MOBILITY
                    score+=count_bits(get_bishop_attacks(square, occupancies[BOTH]));

                    break;
                case R:
                    score+=rook_score[square];
                    if(bitboards[P] & file_masks[square] == 0)
                    {
                        score+=semi_open_file_bonus;
                    }
                    if(((bitboards[P] | bitboards[p]) & file_masks[square] == 0))
                    {
                        score+=open_file_bonus;
                    }
                 break;
                case Q:  
                    score += count_bits(get_queen_attacks(square, occupancies[BOTH]));
                    break;
                case K: 
                    score+=king_score[square];
                    if(bitboards[P] & file_masks[square] == 0)
                    {
                        score-=semi_open_file_bonus;
                    }
                    if((bitboards[P] | bitboards[p]) & file_masks[square] == 0)
                    {
                        score-=open_file_bonus;
                    }
                    score += count_bits(KING_ATTACKS[square] & occupancies[WHITE])*king_safety_bonus;    
                    break;
                case p: 
                    score-=pawn_score[mirror_score[square]]; 
                    double_pawns = count_bits(bitboards[p] & file_masks[square]);
                    if(double_pawns>1)
                    {
                        score-=double_pawns*double_pawn_penalty;
                    }
                    if((bitboards[p] & isolated_masks[square])==0)
                    {
                        score-=isolated_pawn_penalty;
                    }
                    if((black_passed_masks[square] & bitboards[P])==0)
                    {
                        score-=passed_pawn_bonus[(mirror_score[square])%8];
                    }
                    break;
                case n: score-=knight_score[mirror_score[square]];break;
                case b: 
                    score-=bishop_score[mirror_score[square]];
                    //MOBILITY
                    score-=count_bits(get_bishop_attacks(square, occupancies[BOTH]));

                    break;
                case r: 
                    score-=rook_score[mirror_score[square]];
                    if(bitboards[p] & file_masks[square] == 0)
                    {
                        score-=semi_open_file_bonus;
                    }
                    if((bitboards[P] | bitboards[p]) & file_masks[square] == 0)
                    {
                        score-=open_file_bonus;
                    }
                    break;
                case q:
                    score -= count_bits(get_queen_attacks(square, occupancies[BOTH]));
                    break;
                case k: 
                    score-=king_score[mirror_score[square]];
                    if(bitboards[p] & file_masks[square] == 0)
                    {
                        score+=semi_open_file_bonus;
                    }
                    if((bitboards[P] | bitboards[p]) & file_masks[square] == 0)
                    {
                        score+=open_file_bonus;
                    }
                    score -= count_bits(KING_ATTACKS[square] & occupancies[BLACK])*king_safety_bonus;
                    
                    break;
                
            }
            CLEARBIT(bitboard,square);
        }
    }

    if(side==WHITE)
    {
        return score;
    }else return -score;
}
int ply;
int best_move;



tt transposition_table[hash_size];

void clear_transposition_table()
{
    for(int index=0;index<hash_size;index++)
    {
        transposition_table[index].hash_key = 0ULL;
        transposition_table[index].depth = 0;
        transposition_table[index].score = 0;
        transposition_table[index].flag = 0;
    }
}

static inline int probe_tt(int alpha, int beta, int depth)
{
    tt *entry = &transposition_table[hash_key % hash_size];
    if(hash_key == entry->hash_key)
    {
        if(entry->depth >= depth)
        {
            int score = entry->score;
            if(score < -mate_score){score+=ply;}
            if(score > mate_score){score-=ply;}
            switch(entry->flag)
            {
                case hash_flag_exact: return score;
                case hash_flag_alpha: if(score<=alpha) return alpha; break;
                case hash_flag_beta: if(score>=beta) return beta; break;
            }
        }
    }
    return no_hash_entry;   
}
static inline void store_tt(int score, int depth, int hash_flag)
{
    tt *entry = &transposition_table[hash_key % hash_size];

    if(score < -mate_score){score-=ply;}
    if(score > mate_score){score+=ply;}

    entry->hash_key = hash_key;
    entry->score = score;
    entry->depth = depth;
    entry->flag = hash_flag;
}



static inline void enable_pv_scoring(moves *move_list)
{
    follow_pv = 0;
    for(int count=0;count<move_list->move_count;count++)
    {
        if(move_list->moves[count]==pv_table[0][ply])
        {
            score_pv = 1;
            follow_pv = 1;
        }
    }
}

static inline int score_move(int move)
{
    if(score_pv)
    {
        if(pv_table[0][ply]==move)
        {
            score_pv = 0;
            return 20000;
        }
    }

    if(GET_CAPTURE(move))
    {
            int target_piece = P;
            int start_piece, end_piece;
            if (side == WHITE) { start_piece = p; end_piece = k;}
            else{start_piece = P;end_piece = K;}

            for (int bb_piece = start_piece; bb_piece <= end_piece; bb_piece++)
            {
                if (GETBIT(bitboards[bb_piece], TOSQ(move)))
                {
                    target_piece = bb_piece;
                    break;
                }
            }
        return mvv_lva[GET_PIECE(move)][target_piece] + 10000;
    }
    else
    {
        if(killer_moves[0][ply]==move)
        {return 9000;}
        if(killer_moves[1][ply]==move)
        {return 8000;}
        return history_moves[GET_PIECE(move)][TOSQ(move)];
    }
    return 0;
}
static inline int sort_moves(moves *move_list)
{
    int move_scores[move_list->move_count];
    for(int count=0;count<move_list->move_count;count++)
    {
        move_scores[count] = score_move(move_list->moves[count]);
    }
    for (int current_move = 0; current_move < move_list->move_count; current_move++)
    {
        for (int next_move = current_move + 1; next_move < move_list->move_count; next_move++)
        {
            if (move_scores[current_move] < move_scores[next_move])
            {
                int temp_score = move_scores[current_move];
                move_scores[current_move] = move_scores[next_move];
                move_scores[next_move] = temp_score;
        
                int temp_move = move_list->moves[current_move];
                move_list->moves[current_move] = move_list->moves[next_move];
                move_list->moves[next_move] = temp_move;
            }
        }
    }
}
void print_move_scores(moves *move_list)
{
for(int count=0;count<move_list->move_count;count++)
        {
            print_move(move_list->moves[count]);
            cout<<" ";
            cout<<"score: "<<score_move(move_list->moves[count])<<"\n";
        }
}

static inline int is_repetition()
{
    for(int index = 0;index<repetition_index;index++)
    {
        if(repetition_table[index]==hash_key)
        {
            return 1;
        }
    }


    return 0;
}

static inline int quiescence(int alpha, int beta)
{

    if((nodes&2047)==0)
    {
        communicate();
    }


    nodes++;
    if(ply > MAX_PLY - 1)
    {
        return evaluate();
    }



    int evaluation = evaluate();

    if(evaluation>=beta)
    {
        return beta;
    }
    if(evaluation>alpha)
    {
        alpha = evaluation;
    }

    moves move_list[1];

    generate_moves(move_list);

    sort_moves(move_list);
    for(int count=0;count<move_list->move_count;count++)
    {
        COPY_BOARD();

        ply++;
        repetition_index++;
        repetition_table[repetition_index] = hash_key;

        if(make_move(move_list->moves[count],captures)==0)
        {
            ply--;
            repetition_index--;
            continue;
        }
        int score = -quiescence(-beta,-alpha);
        ply--;
        repetition_index--;
        RESET_BOARD();
        if(stopped == 1)
        {
            return 0;
        }   
        if(score>alpha)
        {
            alpha = score;
            if(score>=beta)
            {
                return beta;
            }
        }
    }
    return alpha;
}

const int full_depth_moves = 4;
const int reduction_limit = 3;

static inline int negamax(int alpha, int beta, int depth)
{
    int score;
    int hash_flag = hash_flag_alpha;
    int pv_node = (beta - alpha > 1);

    if(ply && is_repetition())
    {
        return 0;
    }


    if(ply && (score = probe_tt(alpha,beta,depth))!=no_hash_entry && pv_node == 0)
    {
        return score;
    }


    if((nodes&2047)==0)
    {
        communicate();
    }

    pv_length[ply] = ply;
    if(depth==0)
    {
        return quiescence(alpha,beta);
    }
    if(ply > MAX_PLY - 1)
    {
        return evaluate();
    }
    nodes++;

    int in_check = is_square_attacked((side==WHITE)?lsb(bitboards[K]):lsb(bitboards[k]),side^1);
    if(in_check)
    {
        depth++;
    }

    int legal_moves = 0;

    if(depth >= 3 && in_check == 0 && ply)
    {
        COPY_BOARD();
        ply++;
        repetition_index++;
        repetition_table[repetition_index] = hash_key;
        if(enpassant!=no_sqr)
        {
            hash_key ^= enpassant_keys[enpassant];
        }
        enpassant = no_sqr;
        side^=1;
        hash_key ^= side_key;
        score= -negamax(-beta,-beta+1,depth-1-2);
        ply--;
        repetition_index--;
        RESET_BOARD();
        
        // if(stopped = 1){return 0;}
        if(score>=beta)
        {
            return beta;
        }
    }



    moves move_list[1];
    generate_moves(move_list);
    if(follow_pv)
    {
        enable_pv_scoring(move_list);
    }
    sort_moves(move_list);
    int moves_searched = 0  ;
    for(int count=0;count<move_list->move_count;count++)
    {
        COPY_BOARD();

        ply++;
        repetition_index++;
        repetition_table[repetition_index] = hash_key;

        if(make_move(move_list->moves[count],all_moves)==0)
        {
            ply--;
            repetition_index--;
            continue;
        }
        legal_moves++;

    
    
        if(moves_searched == 0)
        {
            score = -negamax(-beta,-alpha,depth-1);
        }
        else 
        {
            if(moves_searched >= full_depth_moves && depth >= reduction_limit && in_check == 0 && GET_CAPTURE(move_list->moves[count]) == 0 && GET_PROMOTED(move_list->moves[count])==0)
            {
                score = -negamax(-alpha-1,-alpha,depth-2);
            }
            else
            {
                score = alpha+1;
            }
            if(score>alpha)
            {
                score = -negamax(-alpha-1,-alpha,depth-1);
                if(score>alpha && score<beta)
                {
                    score = -negamax(-beta,-alpha,depth-1);
                }
            }
        }
            

        
        // score = -negamax(-beta,-alpha,depth-1);
        ply--;
        repetition_index--;
        RESET_BOARD();
        if(stopped == 1)
        {
            cout<<"stopped\n";
            return 0;
        }
        moves_searched++;
        
        if(score>alpha)
        {
            hash_flag = hash_flag_exact;
            if(GET_CAPTURE(move_list->moves[count])==0)
            {
                history_moves[GET_PIECE(move_list->moves[count])][TOSQ(move_list->moves[count])]+=depth;
            }
            alpha = score;
            pv_table[ply][ply] = move_list->moves[count];
            for(int next_ply = ply+1;next_ply<pv_length[ply+1];next_ply++)
            {
                pv_table[ply][next_ply] = pv_table[ply+1][next_ply];
            }
            pv_length[ply] = pv_length[ply+1];
            if(score>=beta)
            {
                store_tt(beta,depth,hash_flag_beta);
                if(GET_CAPTURE(move_list->moves[count])==0)
                {
                    killer_moves[1][ply] = killer_moves[0][ply];
                    killer_moves[0][ply] = move_list->moves[count]; 
                }
                return beta;
            }
        }
    }
    if(legal_moves==0)
    {
        if(in_check)
        {
            return -mate_value+ply;
        }else return 0;
    }
    store_tt(alpha,depth,hash_flag);
    return alpha;
}


void parse_fen(char *fen)
{
    char *tfen = fen;
    memset(bitboards, 0ULL, sizeof(bitboards));
    memset(occupancies, 0ULL, sizeof(occupancies));
    side = 0;
    enpassant = no_sqr;
    castle = 0;

    hash_key = 0ULL;

    repetition_index = 0;

    memset(repetition_table, 0ULL, sizeof(repetition_table));


    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            int square = rank * 8 + file;
            if (*fen >= 'a' && *fen <= 'z' || *fen >= 'A' && *fen <= 'Z')
            {
                int piece = char_pieces[*fen];
                SETBIT(bitboards[piece], square);
                fen++;
            }
            if (*fen >= '0' && *fen <= '9')
            {
                int offset = *fen - '0';
                int piece = -1;
                for (int count = 0; count < 12; count++)
                {
                    if (GETBIT(bitboards[count], square))
                    {
                        piece = count;
                    }
                }
                if (piece == -1)
                {
                    file--;
                }

                file += offset;

                fen++;
            }
            if (*fen == '/')
            {
                fen++;
            }
        }
    }
    fen++;

    (*fen == 'w') ? (side = WHITE) : (side = BLACK);

    fen += 2;

    while (*fen != ' ')
    {
        switch (*fen)
        {
        case 'K':
            castle |= WK;
            break;
        case 'Q':
            castle |= WQ;
            break;
        case 'k':
            castle |= BK;
            break;
        case 'q':
            castle |= BQ;
            break;
        default:
            break;
        }
        fen++;
    }

    fen++;
    if (*fen != '-')
    {
        int file = fen[0] - 'a';
        int rank = 8 - (fen[1] - '0');
        enpassant = rank * 8 + file;
    }
    else
        enpassant = no_sqr;

    occupancies[WHITE] = occupancies[WHITE] | bitboards[P] | bitboards[N] | bitboards[B] | bitboards[R] | bitboards[Q] | bitboards[K];

    occupancies[BLACK] = occupancies[BLACK] | bitboards[p] | bitboards[n] | bitboards[b] | bitboards[r] | bitboards[q] | bitboards[k];

    occupancies[BOTH] = occupancies[WHITE] | occupancies[BLACK];

    hash_key = generate_hash_key();

    cout << "FEN: " << tfen << "\n";
}

void parse_position(char *command)
{
    command += 9;
    char *current_char = command;
    if (strncmp(command, "startpos", 8) == 0)
    {
        char startfen[] = START_FEN;
        parse_fen(startfen);
    }
    else
    {
        current_char = strstr(command, "fen");

        if (current_char == NULL)
        {
            char startfen[] = START_FEN;
            parse_fen(startfen);
        }
        else
        {
            current_char += 4;
            parse_fen(current_char);
        }
    }

    current_char = strstr(command, "moves");
    if (current_char != NULL)
    {

        current_char += 6;

        while (*current_char && *current_char != ' ')
        {
            cout << current_char << "\n";
            int move = parse_move(current_char);
            if (move == 0)
            {
                break;
            }
            repetition_index++;
            repetition_table[repetition_index] = hash_key;
            make_move(move, all_moves);

            while (*current_char && *current_char != ' ')
            {
                current_char++;
            }

            if (*current_char)
            {
                current_char++;
            }
        }
    }
    print_board();
}

void search_position(int depth)
{
    int score = 0;

    nodes = 0;

    stopped = 0;

    follow_pv = 0;

    score_pv = 0;

    memset(killer_moves,0,sizeof(killer_moves));
    memset(history_moves,0,sizeof(history_moves));
    memset(pv_table,0,sizeof(pv_table));
    memset(pv_length,0,sizeof(pv_length));
    int alpha = -inf;
    int beta = inf;

    for(int current_depth =1; current_depth<=depth; current_depth++)
    {

        if(stopped == 1)
        {
            cout<<"search stopped\n";
            break;
        }


        follow_pv = 1;



        score = negamax(alpha,beta,current_depth);

        if((score<=alpha) || (score>=beta))
        {
            alpha = -inf; 
            beta = inf;
            continue;
        }
        alpha = score - valWindow;
        beta = score + valWindow;

        if(score > -mate_value && score < -mate_score)
        {
            cout<<"info depth "<<current_depth<<" score cp "<<-(score + mate_value)/2 - 1<<" nodes "<<nodes<<" time "<<get_time_ms()-starttime<<" pv ";
        }
        else if(score > mate_score && score < mate_value)
        {
            cout<<"info depth "<<current_depth<<" score cp "<<(mate_value-score)/2 + 1<<" nodes "<<nodes<<" time "<<get_time_ms()-starttime<<" pv ";
        }
        else
        {
            cout<<"info depth "<<current_depth<<" score mate "<<score<<" nodes "<<nodes<<" time "<<get_time_ms()-starttime<<" pv ";
        }
        for(int count=0;count<pv_length[0];count++)
        {
            print_move(pv_table[0][count]);
            cout<<" ";
        }
        cout<<"\n";
        
    }
    cout<<"bestmove ";
    print_move(pv_table[0][0]);
    cout<<"\n";
}





void parse_go(char *command)
{
    int depth = -1;

    // init parameters
    // init argument
    char *argument = NULL;

    // infinite search
    if ((argument = strstr(command,"infinite"))) {}

    // match UCI "binc" command
    if ((argument = strstr(command,"binc")) && side == BLACK)
     {   // parse black time increment
        inc = atoi(argument + 5);
     }
    // match UCI "winc" command
    if ((argument = strstr(command,"winc")) && side == WHITE)
     {   // parse white time increment
        inc = atoi(argument + 5);
     }
    // match UCI "wtime" command
    if ((argument = strstr(command,"wtime")) && side == WHITE)
     {   // parse white time limit
        timee = atoi(argument + 6);
     }
    // match UCI "btime" command
    if ((argument = strstr(command,"btime")) && side == BLACK)
    {    // parse black time limit
        timee = atoi(argument + 6);
    }
    // match UCI "movestogo" command
    if ((argument = strstr(command,"movestogo")))
    {    // parse number of moves to go
        movestogo = atoi(argument + 10);
    }
    // match UCI "movetime" command
    if ((argument = strstr(command,"movetime")))
    {
        movetime = atoi(argument + 9);
    }
    // match UCI "depth" command
    if ((argument = strstr(command,"depth")))
    {
        depth = atoi(argument + 6);
        cout<<"depth: "<<depth<<"\n";
    }
    else
        depth = -1;

    // if move time is not available
    if(movetime != -1)
    {
        // set time equal to move time
        timee = movetime;

        // set moves to go to 1
        movestogo = 1;
    }

    // init start time
    starttime = get_time_ms();

    // init search depth
    depth = depth;
    // if time control is available
    if(timee != -1)
    {
        // flag we're playing with time control
        timeset = 1;

        // set up timing
        timee /= movestogo;
        timee -= 50;
        stoptimee = starttime + timee + inc;
    }

    // if depth is not available

    if(depth == -1)
    {
        depth = 64;
    }

    // print debug info
    std::cout << "time:" << timee << " start:" << starttime << " stop:" << stoptimee << " depth:" << depth << " timeset:" << timeset << std::endl;

    // search position
    search_position(depth);
}

void uci_loop()
{
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    char input[2000];
    cout<<"id name Blackflight 0.1\n";
    cout<<"id author Aman Mehrishi\n";
    cout<<"uciok\n";

    while(1)
    {
        memset(input,0,sizeof(input));

        fflush(stdout);

        if(!fgets(input,2000,stdin))
        {
            continue;
        }
        if(input[0]=='\n')
        {
            continue;
        }
        else if(!strncmp(input,"isready",7))
        {
            cout<<"readyok\n";
            continue;
        }
        else if(!strncmp(input,"position",8))
        {
            parse_position(input);
            clear_transposition_table();
        }
        else if(!strncmp(input,"ucinewgame",10))
        {
            char pos[]= "position startpos";
            parse_position(pos);
            clear_transposition_table();
        }
        else if(strncmp(input,"go",2)==0)
        {
            parse_go(input);
        }
        else if(!strncmp(input,"quit",4))
        {
            break;
        }
        else if(!strncmp(input,"uci",3))
        {
            cout<<"id name Blackflight 0.1\n";
            cout<<"id author Aman Mehrishi\n";
            cout<<"uciok\n";
        }
    }


}
void init_all()
{
    init_leaper_attacks();

    init_slider_attacks(BISHOP);

    init_slider_attacks(ROOK);


    // init_magic_numbers();

    init_random_keys(); 
        
    clear_transposition_table();

    init_evaluation_masks();


}


// MAIN DRIVER FUNCTION
int main()
{

    // INITIALIZE LEAPER ATTACKS
    init_all();
    int debug=0;
    if(debug)
    {
        
        char fen[]= "8/8/5Q2/4K3/2k5/8/6B1/8 w - - 11 77 ";
        char mate_fen[] = "6k1/ppppprbp/8/8/8/8/PPPPPRBP/6K1 w - - 0 1 ";
        char start_fen[] = START_FEN;
        char tricky_fen[] = TRICKY_FEN;
        parse_fen(mate_fen);
        print_board(); 
        cout<<"score: "<<evaluate()<<"\n"; 

        // perft_test(6);

        
        
        // search_position(7); 

        // moves move_list[1];
        // generate_moves(move_list);
        // history_moves[GET_PIECE(move_list->moves[0])][TOSQ(move_list->moves[0])]=35;

        // print_move_scores(move_list);
        // sort_moves(move_list);
        // cout<<"\n";
        // print_move_scores(move_list);
    }
    // TEST
    else uci_loop();


    return 0;
}