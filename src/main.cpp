// src/main.cpp
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <windows.h>
#include <fstream>
#include <sstream>

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

    // 讀取外部 CSV 檔案並建構圖
    bool loadDataFromCSV(const string& S_filePath) {
        ifstream F_inputFile(S_filePath);
        
        // 檢查檔案是否成功開啟
        if (!F_inputFile.is_open()) {
            cout << "[錯誤]無法開啟檔案：" << S_filePath << "\n";
            cout << "請確認檔案名稱與路徑是否正確。\n";
            return false;
        }

        string S_line;
        
        // 讀取第一行 (標題行)，不需要把它當作資料處理，讀完就放著
        getline(F_inputFile, S_line); 

        // 迴圈讀取接下來的每一行，直到檔案結束
        int N_rowCount = 0;
        while (getline(F_inputFile, S_line)) {
            stringstream SS_row(S_line);
            string S_source, S_target, S_rateStr;
            double D_rate;

            // 根據逗號 ',' 來切割這一行的字串
            getline(SS_row, S_source, ',');
            getline(SS_row, S_target, ',');
            getline(SS_row, S_rateStr, ',');

            // 將切出來的匯率字串轉換成浮點數
            D_rate = stod(S_rateStr);

            // 呼叫寫好的函式，把這筆資料加入圖中
            addExchangeRate(S_source, S_target, D_rate);
            N_rowCount++;
        }

        F_inputFile.close();
        cout << "成功讀取檔案！共載入 " << N_rowCount << " 筆匯率資料。\n";
        return true;
    }
}; 

int main() {
    // 強制將終端機輸出編碼設定為 UTF-8
    SetConsoleOutputCP(CP_UTF8);

    ArbitrageGraph market;

    cout << "=== 系統初始化中 ===\n";
    // 呼叫讀取 CSV 的函式，傳入相對路徑
    bool B_loadSuccess = market.loadDataFromCSV("market_data.csv");

    // 如果檔案讀取失敗，就直接結束程式
    if (!B_loadSuccess) {
        return 1; 
    }

    cout << "\n=== 市場匯率表 ===\n";
    market.printGraph();
    
    cout << "\n=== 執行三角套利偵測 ===\n";
    market.findTriangularArbitrage("TWD");

    return 0;
}