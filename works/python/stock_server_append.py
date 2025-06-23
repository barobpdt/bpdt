@app.route('/stock/detail/<symbol>', methods=['GET'])
def get_stock_detail(symbol):
    """특정 종목의 상세 정보를 반환합니다."""
    try:
        stock = yf.Ticker(symbol)
        info = stock.info
        if not info:
            return jsonify({"error": "상세 정보를 찾을 수 없습니다."}), 404
        detail = {
            "symbol": symbol,
            "shortName": info.get("shortName"),
            "longName": info.get("longName"),
            "sector": info.get("sector"),
            "industry": info.get("industry"),
            "marketCap": info.get("marketCap"),
            "website": info.get("website"),
            "exchange": info.get("exchange"),
            "currency": info.get("currency"),
            "country": info.get("country"),
            "logo_url": info.get("logo_url"),
            "summary": info.get("longBusinessSummary")
        }
        return jsonify(detail)
    except Exception as e:
        return jsonify({"error": f"상세 정보 조회 실패: {str(e)}"}), 500

@app.route('/stock/history/<symbol>/<end_date>', methods=['GET'])
def get_stock_history(symbol, end_date):
    """특정 종목의 최근 1주일간(영업일 기준) 주가 변동 추이를 반환합니다."""
    try:
        # 날짜 검증
        end_dt = datetime.strptime(end_date, '%Y-%m-%d')
        start_dt = end_dt - timedelta(days=7)
        stock = yf.Ticker(symbol)
        hist = stock.history(start=start_dt, end=end_dt + timedelta(days=1))
        if hist.empty:
            return jsonify({"error": "주가 변동 데이터를 찾을 수 없습니다."}), 404
        history = []
        for idx, row in hist.iterrows():
            history.append({
                "date": idx.strftime('%Y-%m-%d'),
                "open": float(row['Open']),
                "high": float(row['High']),
                "low": float(row['Low']),
                "close": float(row['Close']),
                "volume": int(row['Volume'])
            })
        return jsonify({
            "symbol": symbol,
            "history": history
        })
    except Exception as e:
        return jsonify({"error": f"주가 변동 추이 조회 실패: {str(e)}"}), 500

@app.route('/stock/top-volume/<date>', methods=['GET'])
def get_top_volume_stocks(date):
    """지정한 날짜의 거래량 상위 10종목을 반환합니다."""
    try:
        # 대표적인 대형주(미국 S&P500 일부) 심볼 예시 (실제 서비스에서는 전체 리스트를 DB 등에서 관리)
        major_symbols = [
            'AAPL', 'MSFT', 'GOOGL', 'AMZN', 'TSLA', 'NVDA', 'META', 'AMD', 'NFLX', 'INTC',
            'QCOM', 'CSCO', 'AVGO', 'ADBE', 'PYPL', 'PEP', 'COST', 'TMUS', 'TXN', 'AMAT',
            'SBUX', 'BKNG', 'ISRG', 'MDLZ', 'MU', 'LRCX', 'ADI', 'REGN', 'GILD', 'VRTX',
            'ZM', 'MRNA', 'ABNB', 'ROKU', 'CRWD', 'SNOW', 'PLTR', 'SHOP', 'UBER', 'LYFT'
        ]
        results = []
        for symbol in major_symbols:
            data = stock_manager.get_stock_data(symbol, date)
            if 'error' not in data:
                results.append({
                    "symbol": symbol,
                    "volume": data['volume'],
                    "trading_amount": data['trading_amount'],
                    "close": data['close']
                })
        # 거래량 기준 내림차순 정렬 후 상위 10개
        top10 = sorted(results, key=lambda x: x['volume'], reverse=True)[:10]
        return jsonify(top10)
    except Exception as e:
        return jsonify({"error": f"상위 거래량 종목 조회 실패: {str(e)}"}), 500
