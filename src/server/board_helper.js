const PIECE_LOOKUP = require("../tetrominoes").PIECE_LOOKUP;
const utils = require("./utils");
const NUM_COLUMN = utils.NUM_COLUMN;
const NUM_ROW = utils.NUM_ROW;
const SquareState = utils.SquareState;
const AI_TAP_ARR = utils.AI_TAP_ARR;

// Collision function
function pieceCollision(board, x, y, piece) {
  for (let r = 0; r < piece.length; r++) {
    for (let c = 0; c < piece[r].length; c++) {
      // If the square is empty, we skip it
      if (!piece[r][c]) {
        continue;
      }
      // Coordinates of the piece after movement
      let newX = c + x;
      let newY = r + y;

      // If out of bounds on left, right or bottom, say it does collide
      if (newX < 0 || newX >= NUM_COLUMN || newY >= NUM_ROW) {
        return true;
      }
      // If over the top of the board, ignore
      if (newY < 0) {
        continue;
      }
      // Check if it overlaps the board
      if (board[newY][newX] != 0) {
        return true;
      }
    }
  }
  return false;
}

/** Clear all filled lines on a board
 * @returns the number of lines cleared
 */
function clearLines(board) {
  let fullLines = [];
  for (let r = 0; r < NUM_ROW; r++) {
    let isRowFull = true;
    for (let c = 0; c < NUM_COLUMN; c++) {
      if (board[r][c] == SquareState.EMPTY) {
        isRowFull = false;
        break;
      }
    }
    if (isRowFull) {
      fullLines.push(r);
    }
  }
  for (const r of fullLines) {
    // Move down all the rows above it
    for (let y = r; y > 1; y--) {
      for (let c = 0; c < NUM_COLUMN; c++) {
        board[y][c] = board[y - 1][c];
      }
    }
    // Clear out the very top row (newly shifted into the screen)
    for (let c = 0; c < NUM_COLUMN; c++) {
      board[0][c] = SquareState.EMPTY;
    }
  }
  return fullLines.length;
}

function getBoardWithAddedPiece(board, currentRotationPiece, x, y) {
  let tempBoard = JSON.parse(JSON.stringify(board));
  for (let r = 0; r < currentRotationPiece.length; r++) {
    for (let c = 0; c < currentRotationPiece[r].length; c++) {
      // If the square is empty, we skip it
      if (!currentRotationPiece[r][c]) {
        continue;
      }
      // Coordinates of the piece after movement
      let newX = c + x;
      let newY = r + y;

      // If out of bounds, ignore
      if (newX < 0 || newY < 0 || newX >= NUM_COLUMN || newY >= NUM_ROW) {
        continue;
      }
      // Add to board
      tempBoard[newY][newX] = SquareState.FULL;
    }
  }
  const numLinesCleared = clearLines(tempBoard);

  // utils.logBoard(tempBoard);
  return [tempBoard, numLinesCleared];
}

/**
 * Generates a list of possible moves, given a board and a piece. It achieves this by
 * placing it in each possible rotation and each possible starting column, and then
 * dropping it into the stack and letting the result play out.
 */
function getPossibleMoves(startingBoard, currentPieceId, level, shouldLog) {
  const rotationsList = PIECE_LOOKUP[currentPieceId][0];
  const { rangesLeft, rangesRight } = getPieceRanges(currentPieceId, startingBoard, level);
  const STARTING_X = 3;
  const STARTING_Y = currentPieceId == "I" ? -2 : -1;

  const possibilityList = []; // list of [rotationId, xOffset, columnHeightsStr]
  for (
    let rotationIndex = 0;
    rotationIndex < rotationsList.length;
    rotationIndex++
  ) {
    const currentRotationPiece = rotationsList[rotationIndex];
    for (let xOffset = rangesLeft[rotationIndex]; xOffset <= rangesRight[rotationIndex]; xOffset++) {
      const x = STARTING_X + xOffset;
      let y = STARTING_Y;

      // Skip configurations that are out of bounds
      if (pieceCollision(startingBoard, x, -2, currentRotationPiece)) {
        if (shouldLog) {
          console.log(
            `Out of bounds: index ${rotationIndex}, xOffset ${xOffset}`
          );
        }
        continue;
      }

      // Move the piece down until it hits the stack
      while (!pieceCollision(startingBoard, x, y + 1, currentRotationPiece)) {
        y++;
      }

      // Make a new board with that piece locked in
      const [trialBoard, numLinesCleared] = getBoardWithAddedPiece(
        startingBoard,
        currentRotationPiece,
        x,
        y
      );
      const newSurfaceArray = utils.getSurfaceArray(trialBoard);
      const numHoles = utils.getHoleCount(trialBoard);

      // Add the possibility to the list
      if (shouldLog) {
        console.log(
          `Adding possibility [Index ${rotationIndex}, xOffset ${xOffset}], would make surface ${newSurfaceArray}`
        );
      }
      possibilityList.push([
        rotationIndex,
        xOffset,
        newSurfaceArray,
        numHoles,
        numLinesCleared,
        trialBoard,
      ]);
    }
  }
  if (shouldLog) {
    console.log(
      `Result: ${possibilityList.length} possibilities for ${currentPieceId}`
    );
  }
  return possibilityList;
}

function repeatedlyShift(offsetX, board, initialX, initialY, maxGravity, maxArr, rotationsList) {
  const ranges = [];

  for (
    let rotationIndex = 0;
    rotationIndex < rotationsList.length;
    rotationIndex++
  ) {
    const currentRotationPiece = rotationsList[rotationIndex];

    // Search left/right based on offset X
    let rangeCurrent = 0;
    let x = initialX;
    let y = initialY;
    let gravityCounter = maxGravity;
    let arrCounter = 0;
    while (true) {
      // run a 'frame' of gravity, shifting, and collision checking
      console.log("New frame");
      if (arrCounter == 0) {
        if (pieceCollision(board, x + offsetX, y, currentRotationPiece)) {
          console.log(`Breaking due to collision at ${x - 1}, ${y}`);
          break; // We're done, can't go any further left
        }
        x += offsetX;
        rangeCurrent += offsetX;
        console.log("shift left:", rangeCurrent);
        arrCounter = maxArr;
      } else {
        arrCounter--;
      }

      if (gravityCounter == 0) {
        if (pieceCollision(board, x, y + 1, currentRotationPiece)) {
          // Piece would lock in
          console.log(`Piece locking at: ${x},${y + 1}`);
          break;
        }
        y++;
        console.log("shift down:", y);
        gravityCounter = maxGravity;
      } else {
        gravityCounter--;
      }
    }
    console.log("New range:", rangeCurrent);
    utils.logBoard(
      getBoardWithAddedPiece(board, currentRotationPiece, x, y)[0]
    );
    ranges.push(rangeCurrent);
  }
  return ranges;
}

function getPieceRanges(pieceId, board, level) {
  utils.logBoard(board);
  const initialX = 3;
  const initialY = pieceId === "I" ? -2 : -1;
  const maxGravity = utils.GetGravity(level) - 1; // 0-indexed, executes on the 0 frame. e.g. 2... 1... 0(shift).. 2... 1... 0(shift)
  const maxArr = AI_TAP_ARR - 1;
  const rotationsList = PIECE_LOOKUP[pieceId][0];

  // Piece ranges, indexed by rotation index
  const rangesLeft = repeatedlyShift(-1, board, initialX, initialY, maxGravity, maxArr, rotationsList);
  const rangesRight = repeatedlyShift(1, board, initialX, initialY, maxGravity, maxArr, rotationsList);
  return { rangesLeft, rangesRight };
}

const TEST_BOARD = [
  [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
  [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
  [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
  [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
  [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
  [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
  [0, 0, 1, 0, 0, 0, 0, 0, 0, 0],
  [0, 0, 1, 0, 0, 0, 0, 0, 0, 0],
  [0, 0, 1, 0, 0, 0, 0, 0, 0, 0],
  [0, 0, 1, 0, 0, 0, 0, 0, 0, 0],
  [0, 0, 1, 0, 0, 0, 0, 0, 0, 0],
  [0, 0, 1, 0, 0, 0, 0, 0, 0, 0],
  [1, 1, 1, 1, 1, 1, 1, 1, 1, 0],
  [1, 1, 1, 1, 1, 1, 1, 1, 1, 0],
  [1, 1, 1, 1, 1, 1, 1, 1, 1, 0],
  [1, 1, 1, 1, 1, 1, 1, 1, 1, 0],
  [1, 1, 1, 1, 1, 1, 1, 1, 1, 0],
  [1, 1, 1, 1, 1, 1, 1, 1, 1, 0],
  [1, 1, 1, 1, 1, 1, 1, 1, 1, 0],
  [1, 1, 1, 1, 1, 1, 1, 1, 1, 0],
];

// console.log("Piece ranges:", getPieceRanges("O", getBoardAtHeight(11), 19));
// console.log("Piece ranges:", getPieceRanges("O", TEST_BOARD, 19));

function getBoardAtHeight(height) {
  const board = [];
  for (let i = 0; i < NUM_ROW; i++) {
    board.push(
      i < NUM_ROW - height
        ? [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        : [1, 1, 1, 1, 1, 1, 1, 1, 1, 0]
    );
  }
  return board;
}

module.exports = {
  getPossibleMoves,
  getBoardWithAddedPiece,
  pieceCollision,
};
