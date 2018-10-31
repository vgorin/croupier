pragma solidity 0.4.23;

contract Croupier {
  address public croupier;

  mapping(uint256 => Bet) public bets;

  struct Bet {
    address betterA;
    address betterB;
    uint64 amount;
    uint32 timeA;
    uint32 timeB;   // `timeB - timeA` cannot exceed 3 minutes
    uint8 betA;
    uint8 betB;
    uint8 result;   // provided by croupier, zero means no result provided yet
    bool withdrawA; // indicates if better A has withdrawn the funds
    bool withdrawB; // indicates if better B has withdrawn the funds
  }

  constructor() public {
    croupier = msg.sender;
  }

  // called by players
  function bet(uint256 betId, uint8 betValue) public payable {
    require(msg.sender != croupier);
    require(betValue != 0);
    require(betId != 0);

    Bet storage b = bets[betId];
    require(b.betterA == address(0) || b.betterB == address(0));

    if(b.betterA == address(0)) {
      b.betterA = msg.sender;
      b.timeA = uint32(now);
      b.betA = betValue;
      b.amount = uint64(msg.value);
     }
    else {
      require(b.betterA != msg.sender);
      require(b.timeA > now - 3 minutes);
      require(b.amount == msg.value);

      b.betterB = msg.sender;
      b.timeB = uint32(now);
      b.betB = betValue;
    }
  }

  // called by croupier to provide the result
  function submitResult(uint256 betId, uint8 resultValue) public {
    require(msg.sender == croupier);
    require(resultValue != 0);

    Bet storage b = bets[betId];
    require(b.betterA != address(0));
    require(b.betterB != address(0));
    require(b.result == 0);
    require(b.timeB > now - 3 minutes);

    b.result = resultValue;
  }

  // called by player(s) to receive prize
  function withdraw(uint256 betId) public {
    require(msg.sender != croupier);
    require(betId != 0);

    Bet storage b = bets[betId];
    require(b.betterA == msg.sender && !b.withdrawA || b.betterB == msg.sender && !b.withdrawB);

    if(b.betterA == msg.sender) {
      b.withdrawA = true;
      transfer(msg.sender, b.betA, b.betB, b.result, b.amount);
    }
    else {
      b.withdrawB = true;
      transfer(msg.sender, b.betB, b.betA, b.result, b.amount);
    }
  }

  function transfer(address betterA, uint8 betA, uint8 betB, uint8 result, uint64 amount) private {
    require(betA == result || betB != result);

    if(betA == result && betB != result) {
      // withdraw full total amount
      betterA.transfer(2 * amount);
    }
    else {
      // withdraw 1/2 of the total bet amount
      betterA.transfer(amount);
    }
  }

}
