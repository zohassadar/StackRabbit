#include "playout.hpp"
#include "eval.hpp"
#include "utils.hpp"
#include "params.hpp"
#include "../data/canonical_sequences.hpp"

using namespace std;

/** Selects the highest value lock placement using the fast eval function. */
LockPlacement pickLockPlacement(GameState gameState,
                                const EvalContext *evalContext,
                                OUT vector<LockPlacement> &lockPlacements) {
  float bestSoFar = evalContext->weights.deathCoef - 1;
  LockPlacement bestPlacement = {};
  for (auto lockPlacement : lockPlacements) {
    GameState newState = advanceGameState(gameState, lockPlacement, evalContext);
    float evalScore = fastEval(gameState, newState, lockPlacement, evalContext);
    if (evalScore > bestSoFar) {
      bestSoFar = evalScore;
      bestPlacement = lockPlacement;
    }
  }
  maybePrint("\nBest placement: %d %d\n", bestPlacement.rotationIndex, bestPlacement.x - SPAWN_X);
  return bestPlacement;
}


/**
 * Plays out a starting state 10 moves into the future.
 * @returns the total value of the playout (intermediate rewards + eval of the final board)
 */
float playSequence(GameState gameState, const PieceRangeContext pieceRangeContextLookup[3], const int pieceSequence[SEQUENCE_LENGTH], int playoutLength) {
  // Note down the original AI mode to prevent the AI from putting itself in alternate modes to affect the valuations
  AiMode originalAiMode = getEvalContext(gameState, pieceRangeContextLookup).aiMode;

  float totalReward = 0;
  for (int i = 0; i < playoutLength; i++) {
    // Figure out modes and eval context
    const EvalContext evalContextRaw = getEvalContext(gameState, pieceRangeContextLookup);
    const EvalContext *evalContext = &evalContextRaw;
    FastEvalWeights weights = getWeights(evalContext->aiMode);

    // Get the lock placements
    std::vector<LockPlacement> lockPlacements;
    Piece piece = PIECE_LIST[pieceSequence[i]];
    moveSearch(gameState, &piece, evalContext->pieceRangeContext.inputFrameTimeline, lockPlacements);

    if (lockPlacements.size() == 0) {
      return weights.deathCoef;
    }

    // Pick the best placement
    LockPlacement bestMove = pickLockPlacement(gameState, evalContext, lockPlacements);

    // On the last move, do a final evaluation
    if (i == playoutLength - 1) {
      GameState nextState = advanceGameState(gameState, bestMove, evalContext);
      EvalContext contextRaw = *evalContext;
      // In some contexts, override the current aiMode such that the end of a playout is always compared fairly against other playouts
      if (originalAiMode == DIG || originalAiMode == STANDARD){
        contextRaw.aiMode = originalAiMode;
      }
      float evalScore = fastEval(gameState, nextState, bestMove, &contextRaw);
      if (PLAYOUT_LOGGING_ENABLED) {
        gameState = nextState;
        printBoard(gameState.board);
        printf("Best placement: %c %d, %d\n\n", bestMove.piece->id, bestMove.rotationIndex, bestMove.x - SPAWN_X);
        printf("Cumulative reward: %01f\n", totalReward);
        printf("Final eval score: %01f\n", evalScore);
        printf("*** TOTAL= %f ***\n", totalReward + evalScore);
      }
      if (SHOULD_PLAY_PERFECT && totalReward < 0){
        return 0;
      }
      return totalReward + evalScore;
    }

    // Otherwise, update the state to keep playing
    int oldLines = gameState.lines;
    gameState = advanceGameState(gameState, bestMove, evalContext);
    FastEvalWeights rewardWeights = evalContext->aiMode == DIG ? getWeights(STANDARD) : weights; // When the AI is digging, still deduct from the overall value of the sequence at standard levels
    totalReward += getLineClearFactor(gameState.lines - oldLines, rewardWeights, evalContext->shouldRewardLineClears);
    if (PLAYOUT_LOGGING_ENABLED) {
      printBoard(gameState.board);
      printf("Best placement: %c %d, %d\n\n", bestMove.piece->id, bestMove.rotationIndex, bestMove.x - SPAWN_X);
    }
  }
  return -1; // Doesn't reach here, always returns from i == 9 case
}


float getPlayoutScore(GameState gameState, const PieceRangeContext pieceRangeContextLookup[3], int firstPieceIndex){
  // // Don't perform playouts if logging is enabled
  // if (LOGGING_ENABLED) {
  //   return 0;
  // }

  // Index into the sequences based on the last known piece given by the in-game randomizer.
  // The piece RNG is dependent on the previous piece, we will then have 1000 sequences with accurate RNG given the last known piece
  int pieceOffset = 1000 + firstPieceIndex * 1000;

  float shortPlayoutScore = 0;
  for (int i = 0; i < NUM_PLAYOUTS; i++) {
    // Do one playout
    const int *pieceSequence = canonicalPieceSequences + (pieceOffset + i) * SEQUENCE_LENGTH; // Index into the mega array of piece sequences;
    float playoutScore = playSequence(gameState, pieceRangeContextLookup, pieceSequence, PLAYOUT_LENGTH);
    shortPlayoutScore += playoutScore;
  }

  if (PLAYOUT_RESULT_LOGGING_ENABLED) {
    printf("PlayoutScore %.1f\n", shortPlayoutScore / NUM_PLAYOUTS);
  }
  return NUM_PLAYOUTS == 0 ? 0 : (shortPlayoutScore / NUM_PLAYOUTS);
}




/*
   Unused code block for time-based playouts

 #include <iostream>
   using namespace std::chrono;


   int numPlayoutsShort = 0;
   high_resolution_clock::time_point t1 = high_resolution_clock::now();
   for (int i = 0; i < 1000; i++) {
   // Do one playout
   const int *pieceSequence = canonicalPieceSequences + (offset + i) * SEQUENCE_LENGTH;  // Index into the mega array of piece sequences;
   float playoutScore = playSequence(gameState, pieceRangeContextLookup, pieceSequence, PLAYOUT_LENGTH);
   shortPlayoutScore += playoutScore;
   numPlayoutsShort += 1;

   // Check the elapsed time
   high_resolution_clock::time_point t2 = high_resolution_clock::now();
   duration<double, std::milli> time_span = t2 - t1;

   if (time_span.count() > PLAYOUT_TIME_LIMIT_MS) {
    break;
   }
   }
   // printf("    num playouts %d \n", numPlayoutsShort);
 */
