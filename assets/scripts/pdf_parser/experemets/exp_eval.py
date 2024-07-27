import pandas as pd

dafa = pd.DataFrame({"a": [1, 2, 3, 4], "b": ["aa", "bb", "cc", "dd"]})

print(dafa)

result: list[str | int | float] = []

for i in range(len(dafa.index)):
    exec('result.append(str(df["a"][i]) + df["b"][i])', {"df": dafa, "i": i, "result": result})

print(result)
