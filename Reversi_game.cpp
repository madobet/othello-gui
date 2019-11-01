#include "Reversi_game.h"

const int Reversi_game::BLACK = -1;
const int Reversi_game::WHITE = 1;
const int Reversi_game::EMPTY = 0;
const int Reversi_game::DRAW = 3;
const int Reversi_game::PLAYING = 0;
const int Reversi_game::INVALID = -9;
const int Reversi_game::DIRSET[BOARD_LEN][2] = { { -1,-1 },{ -1,0 },{ -1,1 },{ 0,-1 },{ 0,1 },{ 1,-1 },{ 1,0 },{ 1,1 } };
/*
 * 2(-1,1 )|4(0,1 )|7(1,1 )
 * -----------------------
 * 1(-1,0 )|       |6(1,0 )
 * ------------------------
 * 0(-1,-1)|3(0,-1)|5(1,-1)
 */
Reversi_game::Reversi_game():
    game_status(PLAYING), curr_round(BLACK){
    curr_position[Posof(3, 3)] = curr_position[Posof(4, 4)] = WHITE;
    curr_position[Posof(3, 4)] = curr_position[Posof(4, 3)] = BLACK;
    UpdateMovesSet();
}

int Reversi_game::PrintBoard() {
    int board_tmp[BOARD_SIZE] = {};
    for (int i = 0; i < BOARD_SIZE; i++) board_tmp[i] = curr_position[i];
    for ( const auto &it : moves_set ) board_tmp[it.move_pos] = 2;
    std::cout << "  0 1 2 3 4 5 6 7 " << std::endl;
    std::cout << " |----------------|" << std::endl;
    for (int i = 0; i < BOARD_LEN; i++) {
        std::cout << i << "|";
        for (int j = 0; j < BOARD_LEN; j++) {
            int val = board_tmp[Posof(i, j)];
            if (val == BLACK) std::cout << "@ ";
            else if (val == WHITE) std::cout << "O ";
            else if (val == 2) std::cout << "* ";
            else std::cout << "  ";
        }
        std::cout << "|" << i << std::endl;
    }
    std::cout << " |----------------|" << std::endl;
    std::cout << "  0 1 2 3 4 5 6 7 " << std::endl;
    return 0;
}

int Reversi_game::Move(One_move move_obj) {
    // uppdate the positon
    curr_position[move_obj.move_pos] = curr_round;

    // reverse chesses accordingly
    for (int j = 0; j < BOARD_LEN; j++) {
        for (int k = 0; k < move_obj.rev_n_of_dir[j]; k++)
            curr_position[move_obj.move_pos + Posof(DIRSET[j][0], DIRSET[j][1]) * (k + 1)] *= -1;
    }
    //curr_round == WHITE ? curr_round = BLACK : curr_round = WHITE;
    curr_round = -curr_round;   // reverse the curr_round

    UpdateMovesSet();

    // check which curr_round it is and if the game is over
    if (moves_set.empty()) { // that means we have to change the round
        //curr_round == WHITE ? curr_round = BLACK : curr_round = WHITE;
        curr_round = -curr_round;
        UpdateMovesSet();
        if (moves_set.empty()) { // the competitor also can not move
            int res = 0;
            for (int i = 0; i < BOARD_SIZE; i++) res += curr_position[i];
            if (res * BLACK > 0) game_status = BLACK;
            else if (res * WHITE > 0) game_status = WHITE;
            else game_status = DRAW;
        }
    }
    return game_status;
}

int Reversi_game::Move(int pos) {
    for (int i = 0; i < moves_set.size(); i++) {
        if (moves_set.at(i).move_pos == pos) {
            curr_position[pos] = curr_round;
            return Move(moves_set.at(i));
        }
    }
    return INVALID;
}

int Reversi_game::RandMove() {
    if (game_status != PLAYING) return game_status;
    static std::default_random_engine rand_e;
    static std::uniform_int_distribution<unsigned> dist(0,moves_set.size());
    return Move(moves_set.at(dist(rand_e)));
}

void Reversi_game::UpdateMovesSet() {
    moves_set.clear();
    for (int pos = 0; pos < BOARD_SIZE; pos++) {    // check all positions
        if (curr_position[pos]) continue;   // if already has piece exist
        bool valid = false;                 // 任意方向上是不是可以落子
        int rev_n_at_curr_pos[BOARD_LEN];
        memset(rev_n_at_curr_pos, 0, sizeof(rev_n_at_curr_pos));
        // check all 8 direction if pieces can be reversed
        for (int j = 0; j < BOARD_LEN; j++) {   // 可以考虑使用并行for
            int curr_dir_x = DIRSET[j][0]; int curr_dir_y = DIRSET[j][1];
            int dynamic_pos_x = Xof(pos);  int dynamic_pos_y = Yof(pos);
            int curr_dir_rev_n = 0;
            while (true) {
                dynamic_pos_x += curr_dir_x; dynamic_pos_y += curr_dir_y;
                if (dynamic_pos_x < 0 || dynamic_pos_x >= BOARD_LEN || dynamic_pos_y < 0 || dynamic_pos_y >= BOARD_LEN) break;
                if (curr_position[Posof(dynamic_pos_x, dynamic_pos_y)] == EMPTY) break;
                else if (curr_position[Posof(dynamic_pos_x, dynamic_pos_y)] == -curr_round) curr_dir_rev_n++;
                else if (curr_position[Posof(dynamic_pos_x, dynamic_pos_y)] == curr_round) {
                    if (curr_dir_rev_n) {
                        rev_n_at_curr_pos[j] = curr_dir_rev_n;
                        valid = true;
                    }
                    break;
                }
            }
        }
        if (valid) moves_set.push_back(One_move(pos, rev_n_at_curr_pos));
    }
}
