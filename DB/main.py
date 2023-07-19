import pandas as pd
from sqlalchemy import create_engine, types
df = pd.read_csv('name_data.csv')

engine = create_engine('postgresql://edgar:genius@localhost:5432/test')

#print(df["data"])
df["data"] = df["data"].replace('\"', "\"'")
#print(df["data"])

for i in range(len(df["data"])):
    a = eval(df["data"][i])
  #  print(a)
    df["data"][i] = a
#print(df["data"])
df.to_sql("depr", 
           engine,
           if_exists="append",
             index = False, 
          dtype = {'name': types.TEXT,
                   'data': types.ARRAY(types.TEXT)}) 