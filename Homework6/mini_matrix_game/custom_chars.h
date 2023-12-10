const byte matrixSize = 8;


byte trophyMatrix[matrixSize][matrixSize] = {
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 1, 1, 1, 1, 0, 1},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
    {0, 0, 1, 1, 1, 1, 0, 0}
};

byte timerChar[matrixSize] = {
	0b01110,
	0b00100,
	0b01110,
	0b10011,
	0b11101,
	0b10001,
	0b01110,
	0b00000
};

byte trophyChar[matrixSize] = {
	0b00000,
	0b11111,
	0b10101,
	0b01110,
	0b00100,
	0b00100,
	0b01110,
	0b11111
};

byte wrenchChar[matrixSize] = {
	0b11011,
	0b10001,
	0b11011,
	0b01110,
	0b00100,
	0b00100,
	0b00100,
	0b01110
};

byte amazedChar[matrixSize] = {
	0b11011,
	0b00000,
	0b01010,
	0b00000,
	0b01110,
	0b01010,
	0b01110,
	0b00000
};

byte explosion1Step[matrixSize] = {
	0b00000,
	0b00000,
	0b00100,
	0b01110,
	0b01110,
	0b00100,
	0b00000,
	0b00000
};

byte explosion2Step[matrixSize] = {
	0b00000,
	0b00100,
	0b01110,
	0b11111,
	0b11111,
	0b01110,
	0b00100,
	0b00000
};

byte explosion3Step[matrixSize] = {
	0b00100,
	0b01110,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b01110,
	0b00100
};

byte fullMatrix[matrixSize] = {
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111
};

byte heartChar[matrixSize] = {
	0b00000,
	0b01010,
	0b11111,
	0b11111,
	0b01110,
	0b00100,
	0b00000,
	0b00000
};

byte skullChar[matrixSize] = {
	0b00000,
	0b01110,
	0b11111,
	0b10101,
	0b11111,
	0b01110,
	0b01010,
	0b00000
};

byte upDownArrows[matrixSize] = {
	0b00100,
	0b01010,
	0b10001,
	0b00000,
	0b00000,
	0b10001,
	0b01010,
	0b00100
};

byte playButton[matrixSize] = {
	0b10000,
	0b11000,
	0b11100,
	0b11111,
	0b11111,
	0b11100,
	0b11000,
	0b10000
};

byte downwardArrow[matrixSize] = {
	0b00000,
	0b00000,
	0b00000,
	0b10001,
	0b01010,
	0b00100,
	0b00000,
	0b00000
};

byte linkedin[matrixSize] = {
	0b00000,
	0b00000,
	0b10000,
	0b00110,
	0b10101,
	0b10101,
	0b10101,
	0b10101
};

byte github[matrixSize] = {
	0b11111,
	0b11111,
	0b10101,
	0b00000,
	0b01010,
	0b00000,
	0b10001,
	0b11111
};