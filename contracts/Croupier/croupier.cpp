#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
using namespace eosio;

// Smart Contract Name: croupier
class croupier : public eosio::contract {
  private:
    void transfer(account_name user, uint8_t bet_a, uint8_t bet_b, uint8_t result, uint64_t amount) {
      // ensure better is eligable for withdraw (won or draw result)
      eosio_assert(bet_a == result || bet_b != result, "better is not eligable for withdraw (lost the bet)");

      // if the result is "win"
      if(bet_a == result && bet_b != result) {
        // withdarw total amount - 2 * amount
        // TODO: implement funds transfer
      }
      // otherwise the result is "draw"
      else {
        // withdraw 1/2 of the total bet amount - amount
        // TODO: implement funds transfer
      }
    }

    /// @abi table
    struct bet {
      uint64_t      bet_id;     // primary key
      account_name  better_a;
      account_name  better_b;
      uint64_t      amount;
      uint32_t      time_a;
      uint32_t      time_b;     // (time_b - time_a) cannot exceed 3 minutes
      uint8_t       bet_a;      // 1 (low) or 2 (high)
      uint8_t       bet_b;      // 1 (low) or 2 (high)
      uint8_t       result;     // 1 (low) or 2 (high)
      bool          withdraw_a; // flag indicating better A has withdrawn funds
      bool          withdraw_b; // flag indicating better B has withdrawn funds

      // primary key
      auto primary_key() const {
        return bet_id;
      }
    };

    // create a multi-index table and support secondary key
    typedef eosio::multi_index<N(bet), bet> bets;


  public:
    using contract::contract;

    /// @abi action
    void bet(account_name _user, uint64_t bet_id, uint8_t bet_value, uint64_t bet_amount) {
      // require that only the owner of an account can use this action
      // or somebody with the account authorization
      require_auth(_user);

      // TODO: check sender is not croupier

      // ensure valid bet - not zero
      eosio_assert(bet_value != 0, "invalid bet value");

      // ensure valid bet ID - not zero
      eosio_assert(bet_id != 0, "invalid bet ID");

      // we access the "bet" table by creating an object of type "bets"
      // as parameters we pass code & scope - _self from the parent contract
      bets _bets(_self, _self); // code, scope

      // we access "bet" table through an iterator
      auto _b = _bets.find(bet_id);

      // if there is no bet record
      if(_b == _bets.end()) {
        // insert new bet (party A)
        _bets.emplace(_self, [&](auto& b) {
          b.bet_id = bet_id;
          b.better_a = _user;
          b.amount = bet_amount;
          b.time_a = now();
          b.bet_a = bet_value;
        });
      }
      // otherwise - verify and update it (party B)
      else {
        // verify that better B didn't bet yet
        eosio_assert(_b->better_b == 0, "better B already bet");

        // verify that better B is different than better A
        eosio_assert(_b->better_a != _user, "better B is equal to better A");

        // ensure better B deposits the same amount as better A
        eosio_assert(_b->amount == bet_amount, "amount must be the same as better A");

        // ensure 3 minutes have not passed from bet A
        eosio_assert(_b->time_a > now() - 180, "3 minutes have passed since better A bet");

        // update the bet (party B)
        _bets.modify(_b, _self, [&](auto& b) {
          b.better_b = _user;
          b.time_b = now();
          b.bet_b = bet_value;
        });
      }
    }

    /// @abi action
    void submit_result(account_name _user, uint64_t bet_id, uint8_t result_value) {
      // require that only the owner of an account can use this action
      // or somebody with the account authorization
      require_auth(_user);

      // TODO: ensure sender is a croupier

      // ensure valid bet ID - not zero
      eosio_assert(bet_id != 0, "invalid bet ID");

      // ensure valid result - not zero
      eosio_assert(result_value != 0, "invalid result value");

      // we access the "bet" table by creating an object of type "bets"
      // as parameters we pass code & scope - _self from the parent contract
      bets _bets(_self, _self); // code, scope

      // we must verify that the bet already exists
      // if the bet is found the iterator variable should not be players.end()
      auto _b = _bets.find(bet_id);
      eosio_assert(_b != _bets.end(), "specified bet doesn't exist");

      // ensure both parties have made their bets
      eosio_assert(_b->better_a != 0 && _b->better_b != 0, "one of the parties (probably B) didn't bet yet");

      // ensure bet result has not been yet submitted
      eosio_assert(_b->result == 0, "bet result has been already submitted");

      // ensure 3 minutes have not passed since better B bet
      eosio_assert(_b->time_b > now() - 180, "3 minutes have passed since better B bet");

      // update betting result
      _bets.modify(_b, _self, [&](auto& b) {
        b.result = result_value;
      });
    }

    /// @abi action
    void withdraw(account_name _user, uint64_t bet_id) {
      // require that only the owner of an account can use this action
      // or somebody with the account authorization
      require_auth(_user);

      // ensure valid bet ID - not zero
      eosio_assert(bet_id != 0, "invalid bet ID");

      // we access the "bet" table by creating an object of type "bets"
      // as parameters we pass code & scope - _self from the parent contract
      bets _bets(_self, _self); // code, scope

      // we must verify that the bet already exists
      // if the bet is found the iterator variable should not be players.end()
      auto _b = _bets.find(bet_id);
      eosio_assert(_b != _bets.end(), "specified bet doesn't exist");

      // ensure sender is one of the betters and didn't make a withdraw
      eosio_assert((_b->better_a == _user && !_b->withdraw_a) || (_b->better_b == _user && !_b->withdraw_b), "wrong better or already withdrawn");

      // if withdraw request comes from better A
      if(_b->better_a == _user) {
        // update withdraw flag
        _bets.modify(_b, _self, [&](auto& b) {
          b.withdraw_a = true;
        });

        // perform the withdraw (transfer funds)
        transfer(_user, _b->bet_a, _b->bet_b, _b->result, _b->amount);
      }
      // otherwise request comes from better B
      else {
        // update withdraw flag
        _bets.modify(_b, _self, [&](auto& b) {
          b.withdraw_b = true;
        });

        // perform the withdraw (transfer funds)
        transfer(_user, _b->bet_b, _b->bet_a, _b->result, _b->amount);
      }
    }
};

// specify the contract name, and export a public action: update
EOSIO_ABI(croupier, (bet)(submit_result)(withdraw))

