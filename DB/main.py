import pandas as pd
from sqlalchemy import create_engine, types
df = pd.read_csv('dump.csv')

engine = create_engine('postgresql://edgar:genius@localhost:5432/test')

#print(df["data"])
#df["data"] = df["data"].replace('\"', "\"'")

#print(df["data"])

for i in range(len(df["data"])):
    a = eval(df["data"][i])
    df["data"][i] = a
for i in range(len(df["depr_data"])):
    if (str(df["depr_data"][i]) == "nan"):
        continue
    #print(df["depr_data"][i])
    b = eval(df["depr_data"][i])
    df["depr_data"][i] = b
#print(df["data"])
df.to_sql("depr", 
           engine,
           if_exists="append",
             index = False, 
          dtype = {'name': types.TEXT,
                   'data': types.ARRAY(types.TEXT),
                   'depr_data': types.ARRAY(types.TEXT)}) 