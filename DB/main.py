import pandas as pd
from sqlalchemy import create_engine, types
df = pd.read_csv('dump2.csv')

engine = create_engine('postgresql://edgar:genius@localhost:5432/test')

#print(df["data"])
#df["data"] = df["data"].replace('\"', "\"'")

#print(df["data"])
packages = set()
index = 0
for i in range(len(df["data"])):
    #print(df["data"][i])
    if (str(df["name"][i]) == "boost"):
        print(type(df["depr_data"][i][0]))
    #     exit() 
    if (str(df["data"][i]) == "nan"):
        continue
    a = eval(df["data"][i])
    df["data"][i] = a
    packages.update(a)

check_packages = set()
for i in range(len(df["depr_data"])):
    if (str(df["depr_data"][i]) == "nan"):
        continue
    b = eval(df["depr_data"][i])
    df["depr_data"][i] = b
    check_packages.update(b)

deprCheck = pd.DataFrame(columns = ["name", "check"])
chk = []
names = []
for i in packages:
    names.append(i)
    if i not in check_packages:
        chk.append(None)
    else:
        chk.append(True)

# print(packages)
# print(check_packages, '\n', len(check_packages))
# exit()
deprCheck['name'] = names
deprCheck['check'] = chk
# df.to_sql("depr", 
#            engine,
#            if_exists="append",
#              index = False, 
#           dtype = {'name': types.TEXT,
#                    'data': types.ARRAY(types.TEXT),
#                    'depr_data': types.ARRAY(types.TEXT)}) 

deprCheck.to_sql("deprcheck", 
           engine,
           if_exists="append",
             index = False, 
          dtype = {'name': types.TEXT,
                   "check": types.BOOLEAN,}) 
