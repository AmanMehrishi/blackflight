# Blackflight Chess Engine

![Blackflight Logo](blackflight_logo.png)

Welcome to Blackflight, a bitboard-based chess engine incorporating Negamax along with quiescence search. Blackflight is currently estimated to have an Elo rating in the range of 2000-2200.

## Introduction

Blackflight is designed to be both simple and strong, offering a compact codebase and efficient algorithms. With its bitboard-based representation and emphasis on simplicity, Blackflight serves as an excellent platform for experimentation and development.

## Features

- Bitboard-based representation for efficient move generation and evaluation.
- Negamax algorithm with alpha-beta pruning for move searching.
- Quiescence search to handle the horizon effect and improve move quality.
- Estimation of Elo rating: 2000-2200.
- Detailed logging for debugging and analysis.
- Evaluation function updates efficiently utilizing Piece Square Tables.
  
## Play against Blackflight

You can engage with Blackflight through its user-friendly terminal interface. Simply clone the repository and run the engine:

```bash
$ git clone https://github.com/AmanMehrishi/blackflight.git
$ cd blackflight
$ ./blackflight
```
Blackflight is also available on Lichess.

## Limitations

While Blackflight supports most standard chess rules, it does not currently enforce the 50-move draw rule. Additionally, it may lack some advanced features found in more complex engines, such as dedicated capture generation or sophisticated move ordering techniques.

## Enhancing Blackflight

There are various avenues for enhancing Blackflight's strength:

- Experiment with parallel search algorithms or bitboard optimizations.
- Refine the evaluation function by incorporating additional features or domain-specific knowledge.
- Implement advanced pruning techniques or heuristic enhancements to improve search efficiency.

## License
Blackflight is licensed under the MIT License.



