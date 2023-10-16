#include "server.h"

std::vector<std::string> pending_trxs = {};

Server::Server() {
    this->clients = {};
};

std::shared_ptr<Client> Server::add_client(std::string id) {
    for (auto& [client, amount] : this->clients)
        if (client->get_id() == id) {
            std::random_device rd;
            std::default_random_engine gen(rd());
            std::uniform_int_distribution dis(0, 9);
            for (size_t i = 0; i < 4; ++i) id += std::to_string(dis(gen));
            break;
        }
    
    std::shared_ptr new_client = std::make_shared<Client>(id, *this);
    clients[new_client] = 5;
    return new_client;
}

std::shared_ptr<Client> Server::get_client(std::string id) const{
    for (auto& [client, amount] : this->clients)
        if (client->get_id() == id)
            return client;
    return nullptr;
}

std::map<std::shared_ptr<Client>, double> Server::get_clients() const { return this->clients; }

double Server::get_wallet(std::string id) const{
    for (auto& [client, amount] : this->clients)
        if (client->get_id() == id)
            return amount;
    return NULL;
}

bool Server::parse_trx(std::string trx, std::string& sender, std::string& receiver, double& value) {
    std::regex re(R"((\w+)-(\w+)-(\d+.\d+))");
    std::smatch results;
    if (std::regex_match(trx, results, re)) {
        sender = results[1].str();
        receiver = results[2].str();
        value = std::stod(results[3].str());
        return true;
    } else {
        throw std::runtime_error("transaction doesn't match the pattern");
        return false;
    }
}

bool Server::add_pending_trx(std::string trx, std::string signature) const {
    std::string sender, receiver;   double value;
    Server::parse_trx(trx, sender, receiver, value);

    if (!crypto::verifySignature(get_client(sender)->get_publickey(), trx, signature) ||
        get_client(receiver) == nullptr || get_wallet(sender) < value)
        return false;

    pending_trxs.push_back(trx);
    return true;
}

size_t Server::mine() {
    std::string mempool{};
    for (auto& trx : pending_trxs)
        mempool += trx;
    
    while (true)
        for (auto [client, amount] : this->clients) {
            size_t nonce = client->generate_nonce();
            std::string final = mempool + std::to_string(nonce);
            std::string hash{crypto::sha256(final)};
            if (hash.substr(0, 10).find("0000") != std::string::npos) {
                this->clients[client] += 6.25;

                for (auto& rtx : pending_trxs) {
                    std::string sender, receiver;   double value;
                    Server::parse_trx(rtx, sender, receiver, value);
                    this->clients[get_client(sender)] -= value;
                    this->clients[get_client(receiver)] += value;
                }
                pending_trxs.clear();

                std::cout << client->get_id() <<std::endl;
                return nonce;
            }
        }
}

void show_wallets(const Server& server) {
 	std::cout << std::string(20, '*') << std::endl;
 	for(const auto& client: server.get_clients())
 		std::cout << client.first->get_id() <<  " : "  << client.second << std::endl;
 	std::cout << std::string(20, '*') << std::endl;
}