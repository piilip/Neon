
#include "Grammar.h"

std::string to_string(GrammarSymbol symbol) {
  switch (symbol) {
  case LEFT_PARAN:
    return "LEFT_PARAN";
  case RIGHT_PARAN:
    return "RIGHT_PARAN";
  case STAR:
    return "STAR";
  case PLUS:
    return "PLUS";
  case MINUS:
    return "MINUS";
  case DIV:
    return "DIV";
  case SEMICOLON:
    return "SEMICOLON";
  case END_OF_FILE:
    return "END_OF_FILE";
  case EXPRESSION:
    return "EXPRESSION";
  case FACTOR:
    return "FACTOR";
  case FALSE:
    return "FALSE";
  case FLOAT_LIT:
    return "FLOAT_LIT";
  case INT_LIT:
    return "INT_LIT";
  case PROGRAM:
    return "PROGRAM";
  case STATEMENT:
    return "STATEMENT";
  case STATEMENTS:
    return "STATEMENTS";
  case SUM:
    return "SUM";
  case TERM:
    return "TERM";
  case TRUE:
    return "TRUE";
  }
}

std::string to_string(StateTransitionType transition) {
  switch (transition) {
  case SHIFT:
    return "SHIFT";
  case GOTO:
    return "GOTO";
  case REDUCE:
    return "REDUCE";
  case ACCEPT:
    return "ACCEPT";
  case ERROR:
    return "ERROR";
  }
}

std::string to_string(StateTransition &action) {
  std::string info = "UNKNOWN";
  if (action.type == StateTransitionType::ACCEPT) {
    info = "ACCEPT";
  } else if (action.type == StateTransitionType::ERROR) {
    info = "ERROR";
  } else if (action.type == StateTransitionType::GOTO) {
    info = "GOTO";
    info += ":" + std::to_string(action.nextStateIndex);
  } else if (action.type == StateTransitionType::SHIFT) {
    info = "SHIFT";
    info += ":" + std::to_string(action.nextStateIndex);
  } else if (action.type == StateTransitionType::REDUCE) {
    info = "REDUCE";
    info += ":" + to_string(action.symbol) + "->";
    for (auto rule : action.rule) {
      info += to_string(rule) + " ";
    }
    if (info[info.length() - 1] == ' ') {
      info = info.substr(0, info.length() - 1);
    }
  }
  std::string result = "Action (" + info + ")";
  return result;
}
