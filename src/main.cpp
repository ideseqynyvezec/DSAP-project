// src/main.cpp
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class ArbitrageGraph {
private:
    // 外層的 Key 是來源貨幣，內層是 (目標貨幣 -> 匯率)
    unordered_map<string, unordered_map<string, double>> M_exchangeRates;
    int N_totalCurrency;

public:
    ArbitrageGraph() : N_totalCurrency(0) {}

    // 新增匯率邊
    void addExchangeRate(const string& sourceCurrency, const string& targetCurrency, double rate) {
        if (M_exchangeRates.find(sourceCurrency) == M_exchangeRates.end()) {
            N_totalCurrency++;
        }
        M_exchangeRates[sourceCurrency][targetCurrency] = rate;
    }

    // 顯示目前的圖結構，用於 Debug
    void printGraph() {
        cout << "目前共有 " << N_totalCurrency << " 種貨幣參與運算。\n";
        for (const auto& sourcePair : M_exchangeRates) {
            for (const auto& targetPair : sourcePair.second) {
                cout << sourcePair.first << " -> " << targetPair.first 
                     << " : " << targetPair.second << "\n";
            }
        }
    }
    
    // 取得內部資料結構
    const unordered_map<string, unordered_map<string, double>>& getGraphData() const {
        return M_exchangeRates;
    }
};

// 基礎 DFS：尋找長度為 3 的三角套利路徑
    void findTriangularArbitrage(const string& startCurrency) {
        if (M_exchangeRates.find(startCurrency) == M_exchangeRates.end()) {
            cout << "找不到起始貨幣：" << startCurrency << "\n";
            return;
        }

        bool B_foundOpportunity = false;
        const auto& graphData = M_exchangeRates;

        // Step 1: 從起點出發到第一層 (Currency A)
        for (const auto& step1 : graphData.at(startCurrency)) {
            string currencyA = step1.first;
            double rate_Start_to_A = step1.second;

            if (graphData.find(currencyA) == graphData.end()) continue;

            // Step 2: 從第一層到第二層 (Currency B)
            for (const auto& step2 : graphData.at(currencyA)) {
                string currencyB = step2.first;
                double rate_A_to_B = step2.second;

                if (graphData.find(currencyB) == graphData.end()) continue;

                // Step 3: 從第二層找尋回到起點的路徑
                for (const auto& step3 : graphData.at(currencyB)) {
                    if (step3.first == startCurrency) {
                        double rate_B_to_Start = step3.second;
                        
                        // 計算循環匯率乘積
                        double finalProfit = rate_Start_to_A * rate_A_to_B * rate_B_to_Start;
                        
                        // 如果大於 1，代表存在套利空間
                        if (finalProfit > 1.0) {
                            cout << "[發現套利機會!] " 
                                 << startCurrency << " -> " << currencyA << " -> " 
                                 << currencyB << " -> " << startCurrency << "\n";
                            cout << "預期利潤率: " << (finalProfit - 1.0) * 100 << "%\n\n";
                            B_foundOpportunity = true;
                        }
                    }
                }
            }
        }

        if (!B_foundOpportunity) {
            cout << "在 " << startCurrency << " 開頭的路徑中，目前沒有發現三角套利空間。\n";
        }
    }

int main() {
    ArbitrageGraph market;

    // 先寫入一些測試假資料 (其中 TWD -> USD -> JPY -> TWD 會大於 1)
    market.addExchangeRate("TWD", "USD", 0.032);
    market.addExchangeRate("USD", "JPY", 145.5);
    market.addExchangeRate("JPY", "TWD", 0.22); // 0.032 * 145.5 * 0.22 = 1.02432 (約 2.4% 利潤)

    // 一條不會賺錢的路徑
    market.addExchangeRate("TWD", "EUR", 0.029);
    market.addExchangeRate("EUR", "GBP", 0.85);
    market.addExchangeRate("GBP", "TWD", 40.0); // 0.029 * 0.85 * 40 = 0.986 (虧損)

    cout << "=== 市場匯率表 ===\n";
    market.printGraph();
    cout << "\n=== 執行三角套利偵測 ===\n";
    
    market.findTriangularArbitrage("TWD");

    return 0;
}
