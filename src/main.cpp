#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <windows.h>
#include <fstream>
#include <sstream>
#include <chrono>
#include <map>

using namespace std;

// 定義單一時間點的市場狀態
struct MarketSnapshot {
    long long L_timestamp;
    // 儲存該時刻的圖：Source -> (Target -> Rate)
    unordered_map<string, unordered_map<string, double>> M_rates;
};

class HistoricalBacktester {
private:
    // 使用 map 確保時間快照按順序排列
    map<long long, MarketSnapshot> M_allSnapshots;

public:
    // 讀取歷史 CSV 檔案
    bool loadHistoricalData(const string& S_filePath) {
        ifstream F_input(S_filePath);
        if (!F_input.is_open()) {
            cout << "[錯誤]找不到檔案：" << S_filePath << "\n";
            return false;
        }

        string S_line;
        getline(F_input, S_line); // 跳過標題列

        int N_rowCount = 0;
        while (getline(F_input, S_line)) {
            if (S_line.empty() || S_line == "\r") continue;
            
            stringstream SS_row(S_line);
            string S_tsStr, S_src, S_dst, S_rateStr;
            
            getline(SS_row, S_tsStr, ',');
            getline(SS_row, S_src, ',');
            getline(SS_row, S_dst, ',');
            getline(SS_row, S_rateStr, ',');

            long long L_ts = stoll(S_tsStr);
            double D_rate = stod(S_rateStr);

            M_allSnapshots[L_ts].L_timestamp = L_ts;
            M_allSnapshots[L_ts].M_rates[S_src][S_dst] = D_rate;
            N_rowCount++;
        }
        
        cout << "成功載入真實數據！共計 " << M_allSnapshots.size() << " 個小時快照。\n";
        return true;
    }

    // 執行全量回測
    void runFullBacktest(const string& S_startCurrency) {
        int N_totalOpportunities = 0;
        double D_maxProfit = 0.0;

        cout << "\n--- 開始歷史回測 (Research Mode) ---\n";
        
        auto start_timer = chrono::high_resolution_clock::now();

        for (auto const& [L_ts, snapshot] : M_allSnapshots) {
            double D_profit = checkArbitrage(snapshot, S_startCurrency);
            if (D_profit > 1.0) {
                N_totalOpportunities++;
                if (D_profit > D_maxProfit) D_maxProfit = D_profit;
                
                // 轉換時間戳記為可讀格式 (簡單處理)
                cout << "[時間戳 " << L_ts << "] 發現機會！收益率: " << (D_profit - 1) * 100 << "%\n";
            }
        }

        auto end_timer = chrono::high_resolution_clock::now();
        chrono::duration<double, milli> D_execTime = end_timer - start_timer;

        cout << "\n========================================\n";
        cout << "分析報告：\n";
        cout << "- 掃描快照數: " << M_allSnapshots.size() << "\n";
        cout << "- 發現套利總次數: " << N_totalOpportunities << "\n";
        cout << "- 歷史最大收益率: " << (D_maxProfit > 0 ? (D_maxProfit - 1) * 100 : 0) << "%\n";
        cout << "- 總執行耗時: " << D_execTime.count() << " 毫秒 (ms)\n";
        cout << "========================================\n";
    }

private:
    // 核心偵測演算法 (DFS 模式)
    double checkArbitrage(const MarketSnapshot& snapshot, const string& S_start) {
        const auto& M_data = snapshot.M_rates;
        if (M_data.find(S_start) == M_data.end()) return 0.0;

        for (auto const& [S_currA, D_rateA] : M_data.at(S_start)) {
            if (M_data.find(S_currA) == M_data.end()) continue;
            for (auto const& [S_currB, D_rateB] : M_data.at(S_currA)) {
                if (M_data.find(S_currB) == M_data.end()) continue;
                if (M_data.at(S_currB).count(S_start)) {
                    double D_totalRate = D_rateA * D_rateB * M_data.at(S_currB).at(S_start);
                    // 考量法幣極高效率，門檻設極低 (1.00001) 進行研究
                    if (D_totalRate > 1.00001) return D_totalRate;
                }
            }
        }
        return 0.0;
    }
};

int main() {
    SetConsoleOutputCP(CP_UTF8);
    HistoricalBacktester backtester;

    // 讀取剛剛 Python 抓取的真實法幣資料
    if (backtester.loadHistoricalData("fiat_historical_rates.csv")) {
        backtester.runFullBacktest("USD");
    }

    return 0;
}