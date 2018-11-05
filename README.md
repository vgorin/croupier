# Environment Setup

To setup new fresh environment:

1. Goto project root (contains "contracts" folder and "docker-compose.yml" file)
2. Run docker-compose:  
  ```docker-compose up -d```
3. Register "cleos" command:  
  ```alias cleos='docker-compose exec keosd /opt/eosio/bin/cleos -u http://nodeosd:8888 --wallet-url http://localhost:8900'```
4. Create a wallet:  
  ```cleos wallet create --file .Pass```
5. Create and import owner and active keys:  
  ```
    cleos create key --file .OwnerKeys
    cleos create key --file .ActiveKeys
    cleos wallet import --private-key=$(cat .OwnerKeys |grep Private |cut -d" " -f3)
    cleos wallet import --private-key=$(cat .ActiveKeys |grep Private |cut -d" " -f3)
  ```
6. Import eosio private key:  
  ```cleos wallet import --private-key=5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3```
7. Create an account:  
  ```
    OwnerKey=$(cat .OwnerKeys |grep Public |cut -d" " -f3)
    ActiveKey=$(cat .ActiveKeys |grep Public |cut -d" " -f3)
    cleos create account eosio ekkis $OwnerKey $ActiveKey
  ```
8. Setup a script to compile and deploy smart contracts:  
  ```
    eosiocpp() { d=/contracts/usr; docker exec eos_nodeosd_1 /opt/eosio/bin/eosiocpp $1 $d/$2 $d/$3; }
    eosbuild() { d=/contracts/usr; eosiocpp -o $1.wast $1.cpp; eosiocpp -g $1.abi $1.cpp; cleos set contract ekkis $d/${1%/*} $d/$1.wast $d/$1.abi; }
  ```

## Environment Cleanup

To remove previously installed environment

1. List docker containers:  
  ```docker ps -a```  
  Example output:  
  ```
    CONTAINER ID        IMAGE               COMMAND                  CREATED             STATUS              PORTS                                            NAMES
    2913edf6b51d        eosio/eos-dev       "/opt/eosio/bin/keos…"   4 hours ago         Up 4 hours                                                           eos_keosd_1
    c5ac6ee688ed        eosio/eos-dev       "/opt/eosio/bin/node…"   4 hours ago         Up 4 hours          0.0.0.0:8888->8888/tcp, 0.0.0.0:9876->9876/tcp   eos_nodeosd_1
  ```
2. Stop the containers  
  ```
    docker stop 2913edf6b51d
    docker stop c5ac6ee688ed
  ```
3. Remove the containers  
  ```
    docker rm 2913edf6b51d
    docker rm c5ac6ee688ed
  ```
4. List data volumes  
  ```docker volume list```  
  Example output:
  ```
    DRIVER              VOLUME NAME
    local               0386c7a90f0aaffb7d563f2cfbe992ec275ab00cc912130da07980705e13cb73
    local               31e288a549ca89d9cfff618505a9fca265efcad9a94a97076e247c7ba7b95122
    local               a3aea26412808113bb558243a2dc8610aec50c71e3d59a8b1e28d617413a9399
    local               d3cb102ce45109200b4bae80dfe656b3683dc75863acfef1dc650564609cd3e0
    local               eos_keosd-data-volume
    local               eos_nodeos-data-volume
  ```
5. Remove **eos_keosd-data-volume** and **eos_nodeos-data-volume** volumes:  
  ```
    docker volume rm eos_keosd-data-volume
    docker volume rm eos_nodeos-data-volume
  ```

# Deployment

1. Compile and deploy smart contracts  
  ```eosbuild Croupier/Croupier```

# Interacting with deployed smart contract

## Setting up test accounts
  **TODO**

1. Making a bet  
  ```cleos push action croupier bet '["ekkis", 1, 1, 10]' -p ekkis@active```
2. Submitting result  
  ```cleos push action croupier submit_result '["ekkis", 1, 1]' -p ekkis@active```
3. Withdrawing funds  
  ```cleos push action croupier withdraw '["ekkis", 1]' -p ekkis@active```
