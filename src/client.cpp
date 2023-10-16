#include "client.h"

Client::Client(std::string id, const Server& server) : id(id), server(&server) {
	crypto::generate_key(this->public_key, this->private_key);
}
std::string Client::get_id() const { return this->id; }
std::string Client::get_publickey() const { return this->public_key; }
double Client::get_wallet() const { return this->server->get_wallet(this->id); }
std::string Client::sign(std::string txt) const{ return crypto::signMessage(this->private_key, txt); }

bool Client::transfer_money(std::string receiver, double value) {
	std::string trx = this->id + '-' + receiver + '-' + std::to_string(value);
	return this->server->add_pending_trx(trx, sign(trx));
}

size_t Client::generate_nonce() const{
	std::random_device rd;
	std::default_random_engine gen(rd());
	std::uniform_int_distribution dis(0, INT_MAX);
	return dis(gen);
}