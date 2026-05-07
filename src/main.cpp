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
