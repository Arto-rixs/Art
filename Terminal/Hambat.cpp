#include <iostream>
#include <vector>
#include <string>

struct PricePair {
    std::string description;
    double time;
    double price;
};

void comparePricePairs(const std::vector<PricePair>& pairs) {
    double minPricePerHour = pairs[0].price / pairs[0].time;
    int minIndex = 0;

    for (int i = 1; i < pairs.size(); ++i) {
        double pricePerHour = pairs[i].price / pairs[i].time;
        if (pricePerHour < minPricePerHour) {
            minPricePerHour = pricePerHour;
            minIndex = i;
        }
    }

    std::cout << "Наиболее выгодная пара: " << std::endl;
    std::cout << "Описание: " << pairs[minIndex].description << std::endl;
    std::cout << "Цена в час: " << pairs[minIndex].time << ", Цена: " << pairs[minIndex].price << std::endl;
}

int main() {
    std::vector<PricePair> pairs = {
        {"Markerts_Fan.tokens(1)", 2014, 807.304},
        {"Markerts_Staking(2)", 1014, 83.440},
        {"Markerts_BTC.pairs(3)", 110, 181.341},
        {"Markerts_ETH.pairs(4)", 103, 99.689},
        {"Markerts_Top.10.cmc.pairs(5)", 168, 42.813},
        {"Markerts_GameFi.tokens(6)", 147, 21.407},
        {"Markerts_Defi2.0.tokens(7)", 79, 11.920},
        {"Markerts_SociaFi.tokens(8)", 98, 11.920},
        {"Markerts_Meme.coins(9)", 232, 85.626},
        {"Markerts_Shit.coins(10)", 1016, 119200},
        {"Markerts_Margi.trading.x10(11)", 541, 59600},
        {"Markerts_Margi.trading.x20(12)", 689, 59600},
        {"Markerts_Margi.trading.x30(13)", 984, 83440},
        {"Markerts_Margi.trading.x50(14)", 2016, 238.399},
        {"Markerts_Derivatised(15)", 974, 59600},
        {"Markerts_Web3.integration(16)", 1035, 55.621},
        {"Markerts_P2P.trading(17)", 670, 35.940},
        {"Markerts_Trading.bots(18)", 335, 17.970},
        {"PR&Team_Support.team(1)", 138, 17.880},
        {"PR&Team_HamsterBook(2)", 138, 11.920},
        {"PR&Team_X(3)", 157, 13.112},
        {"PR&Team_Cointelegraph(4)", 79, 8.344},
        {"PR&Team_HamsterTube(5)", 189, 25.688},
        {"PR&Team_HamsterGram(6)", 105, 21.407},
        {"PR&Team_TikTok(7)", 197, 17.880},
        {"PR&Team_Coindesk(8)", 157, 23.840},
        {"PR&Team_influencers(9)", 568, 107.033},
        {"PR&Team_CEO(10)", 197, 23.840},
        {"PR&Team_IT.team(11)", 472, 47.680},
        {"PR&Team_Marketing(12)", 138, 23.840},
        {"PR&Team_Partnership.program(13)", 138, 11.920},
        {"PR&Team_Product.team(14)", 197, 23.840},
        {"PR&Team_BisDev.team(15)", 98, 11.920},
        {"PR&Team_Two.factor.authentication(16)", 301, 159.841},
        {"PR&Team_Ux.and.UI.team(17)", 368, 32.538},
        {"PR&Team_Security.team(17)", 393, 23.840},
        {"PR&Team_Qa.tea.(18)", 374, 30.396},
        {"PR&Team_Antihacking.shield(19)", 232, 85.626},
        {"PR&Team_Risk.management.team(20)", 521, 47.680},
        {"PR&Team_Security.Audition(20)", 197, 71.520},
        {"PR&Team_Anonymous.transactions.ban(21)", 631, 38.532},
        {"PR&Team_Blacking.suspicius.accounts(22)", 315, 29.800},
        {"PR&Team_Tokenomics.expert(23)", 859, 42.786},
        {"PR&Team_Consensus.Explorer.pass(24)", 1096, 49.498},
        {"PR&Team_VC.labs(25)", 655, 30.689},
        {"PR&Team_Compliance.officer(26)", 157, 6.930},
        {"Legal_KYC(1)", 20, 2.384},
        {"Legal_KYB(2)", 118, 11.920},
        {"Legal_Legal.opinion(3)", 118, 23.840},
        {"Legal_SEC.transparancy(4)", 118, 28.608},
        {"Legal_Anti.money.loundering(5)", 551, 71.520},
        {"Legal_Licence.UAE(6)", 1010, 119.200},
        {"Legal_Licence.Europe(7)", 1041, 119.200},
        {"Legal_Licence.Asia(8)", 636, 42.786},
        {"Legal_Licence.South.America(9)", 626, 27.580},
        {"Legal_Licence.Australia(10)", 1016, 42.786},
        {"Legal_License.North.America(11)", 1076, 139.387},
        {"Legal_License.Nigeria(12)", 292, 12.836},
        {"Legal_Licence.Japan(13)", 3064, 132.665}
    };

    comparePricePairs(pairs);

    return 0;
}
