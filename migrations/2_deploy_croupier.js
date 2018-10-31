const Croupier = artifacts.require("./Croupier");

module.exports = async function(deployer, network, accounts) {
	if(network === "test") {
		console.log("[deploy croupier] test network - skipping the migration script");
		return;
	}
	if(network === "coverage") {
		console.log("[deploy croupier] coverage network - skipping the migration script");
		return;
	}

	await deployer.deploy(Croupier);
};
