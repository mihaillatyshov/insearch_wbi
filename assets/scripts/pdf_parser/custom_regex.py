regex_float = r"(\d+\.\d+)"
regex_M = r"(M\d+)"
regex_B = r"(R\d+)"
regex_R = f"(R{regex_float})"
regex_d_ = r"(-d\d+)"
regex_Rd = f"{regex_R}{regex_d_}"

regex = {}
regex["float"] = r"(\d+\.\d+)"
regex["M"] = r"(M\d+)"
regex["B"] = r"(B\d+)"
regex["T"] = r"(T\d+)"
regex["D"] = r"(D\d+)"
regex["K"] = r"(K\d+)"
regex["CR"] = r"(CR\d+)"
regex["R"] = f"(R{regex['float']})"
regex["R_int"] = r"(R\d+)"
regex["-d"] = r"(-d\d+)"
regex["R-d"] = f"{regex['R']}{regex['-d']}"
regex["mill_M-EI"] = rf"(\d+)(-EI)({regex['float']}|(\d+))M$"
regex["mill_M-I"] = rf"(\d+)(-I){regex['float']}M$"

# "float-MRd": f"^{regex['float']}-{regex['M']}({regex['R-d']}|{regex['R']}|{regex['-d']}|$)$",
full_regex = {
    "float-MRd": f"^{regex['float']}-{regex['M']}{regex['R']}?{regex['-d']}?$",
    "float-BRd": f"^{regex['float']}-{regex['B']}{regex['R']}?{regex['-d']}?$",
    "float-KRd": f"^{regex['float']}-{regex['K']}{regex['R']}?{regex['-d']}?$",
    "float-CRRd": f"^{regex['float']}-{regex['CR']}({regex['R_int']}|{regex['R']}){regex['-d']}?$",
    "float-Rd": f"^{regex['float']}-{regex['R_int']}{regex['-d']}?$",
    "float-T": f"^{regex['float']}-{regex['T']}$",
    "float-D": f"^{regex['float']}-{regex['D']}$",
    "RHC-C?": r"^M(\d+)H7(-C)?$",
    "mill_M-EI": rf"^{regex['mill_M-EI']}",
    "mill_M-I": f"^{regex['mill_M-I']}",
}

page_6_7_10_11_12 = full_regex["float-MRd"]
page_8_9 = full_regex["float-BRd"]

page_13_14_17_18_19 = full_regex["float-MRd"]
page_15_16 = full_regex["float-BRd"]

page_20_21_24_25_26 = full_regex["float-MRd"]
page_22_23 = full_regex["float-BRd"]

page_27_28_31_32 = full_regex["float-MRd"]
page_29_30 = full_regex["float-BRd"]

page_33_34 = full_regex["float-MRd"]
page_35_36 = full_regex["float-BRd"]

page_37_38 = full_regex["float-MRd"]
page_39_40 = full_regex["float-BRd"]

page_41_42 = full_regex["float-MRd"]

page_43 = full_regex["float-MRd"]
page_44 = full_regex["float-BRd"]

page_45_46 = full_regex["float-MRd"]
page_47_48 = full_regex["float-BRd"]

page_49_50 = full_regex["float-T"]

page_51 = full_regex["float-CRRd"]

page_52 = full_regex["float-Rd"]

page_53 = full_regex["float-D"]

page_54 = full_regex["float-KRd"]

page_55 = full_regex["float-MRd"]
