#!/usr/bin/python3

# Python downloader for Yahoo historical data
# Run this once in every blue moon, not really needed.  Will append to old data

import pandas as pd
from datetime import datetime, timedelta
import os,time


#update tickers
# import Merge_Tickers

print("Yahoo Historical Data Downloader")

tickerlist=pd.read_csv('Nasdaq.csv')

tickerlist=tickerlist['Tickers:'].tolist()
tickerlist.sort()
tickerlist=list(map(str.strip,tickerlist))

directory=os.path.join(os. getcwd(),"YahooHistorical")
os.makedirs(directory,exist_ok=True)
os.chdir(directory)


for ticker in tickerlist:
    try:
        timenow=int(datetime.timestamp(datetime.now())) #in s

        data= pd.read_csv("https://query1.finance.yahoo.com/v7/finance/download/{ticker}?period1=0&period2={rightnow}&interval=1d&events=history".format(ticker=ticker,rightnow=timenow))
        data["Date"]=pd.to_datetime(data["Date"]).dt.strftime("%s")
        #if the data is empty, or contains only zeros, ignore it.
        if data.loc[(data!=0).any(1)].empty:
            print("Empty Data Set: "+ticker)
        else:
            #if the file exists, add onto the old data
            if os.path.isfile('{}_yahoo_historical.csv'.format(ticker)):
                pastdata=pd.read_csv('{}_yahoo_historical.csv'.format(ticker))
                #oldest data is on top

                #merge the data
                #cut off any redundant data from the new dataset
                data=data.loc[data["Date"].astype("int")>pastdata["Date"].values[-1]]
                pastdata.append(data).to_csv('{}_yahoo_historical.csv'.format(ticker),header=True,index=False)
            else:
                data.to_csv('{}_yahoo_historical.csv'.format(ticker),header=True,index=False)
            print(ticker)
            time.sleep(.5)

    except:
        print("Failure in downloading ticker: "+ticker)
