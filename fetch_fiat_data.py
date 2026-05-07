import yfinance as yf
import pandas as pd
import time

# 定義你想抓取的法定貨幣對
fiat_pairs = {
    'USDTWD=X': ('USD', 'TWD'),
    'EURUSD=X': ('EUR', 'USD'),
    'EURTWD=X': ('EUR', 'TWD'),
    'USDJPY=X': ('USD', 'JPY'),
    'EURJPY=X': ('EUR', 'JPY')
}

all_data = []

print("開始從 Yahoo Finance 獲取法定貨幣歷史匯率...")

for ticker, (source, target) in fiat_pairs.items():
    print(f"正在下載 {source}/{target}...")
    # 下載過去 1 個月，每 1 小時的歷史報價
    data = yf.download(ticker, period="1mo", interval="1h", progress=False)
    
    if data.empty:
        print(f"警告：抓不到 {ticker} 的資料")
        continue

    # 如果 'Close' 是一個 DataFrame (多層欄位)，我們強制只取第一行
    if isinstance(data['Close'], pd.DataFrame):
        closes = data['Close'].iloc[:, 0]
    else:
        closes = data['Close']

    # 直接從 Series 中遍歷「時間索引 (dt)」與「匯率數值 (rate)」
    for dt, rate in closes.items():
        # 如果遇到空值 (NaN) 就跳過
        if pd.isna(rate): 
            continue
            
        # 直接對索引 dt 轉換為 Timestamp
        ts = int(dt.timestamp())
        rate = float(rate)
        
        # 寫入正向匯率 (Source -> Target)
        all_data.append([ts, source, target, rate])
        # 寫入反向匯率 (Target -> Source)
        all_data.append([ts, target, source, 1.0 / rate])
        
    time.sleep(0.5)

print("\n資料下載完成，正在整理格式...")

# 轉換為 DataFrame 並排序
df = pd.DataFrame(all_data, columns=['Timestamp', 'Source', 'Target', 'Rate'])
df = df.sort_values(by=['Timestamp', 'Source'])

# 匯出為 CSV 檔案
output_filename = "fiat_historical_rates.csv"
df.to_csv(output_filename, index=False)
print(f"大功告成！已匯出至 {output_filename}")