class YG1_Parsers():
    @staticmethod
    def grade_to_app1(grade: str | None) -> str | None:
        if grade is None:
            return None

        return {
            "YG602": "PMKNS",
            "YG712": "P",
            "YG500": "N",
            "YG622": "PK",
            "YG50": "N",
            "YG603": "M",
            "YG501": "K",
            "YG5020": "K",
            "YG501G": "K",
            "YG613": "PM",
            "YG713": "P",
            "YG612": "PMS",
            "YG012": "PH",
            "YG801": "P",
            "YG1001": "PK",
            "YG10": "N",
            "YG3020": "P",
            "YG3010": "P",
            "YG3030": "P",
            "YG100": "N",
            "YG211": "M",
            "YG213": "M",
            "YG214": "M",
            "YG3015": "P",
            "YG1010": "K",
            "YG401": "S",
            "YC0041": "S",
            "YG3115": "P",
            "YG2025": "M",
            "YT100": "PMK",
            "YG812": "PMKS",
        }.get(grade, None)

    @staticmethod
    def grade_to_method(grade: str | None) -> str | None:
        if grade is None:
            return None

        return {
            "YG602": "PVD",
            "YG712": "PVD",
            "YG500": "PVD",
            "YG622": "PVD",
            "YG50": "NONE",
            "YG603": "PVD",
            "YG501": "PVD",
            "YG5020": "CVD",
            "YG501G": "PVD",
            "YG613": "PVD",
            "YG713": "PVD",
            "YG612": "PVD",
            "YG012": "PVD",
            "YG801": "PVD",
            "YG1001": "CVD",
            "YG10": "NONE",
            "YG3020": "CVD",
            "YG3010": "CVD",
            "YG3030": "CVD",
            "YG100": "DLC",
            "YG211": "PVD",
            "YG213": "PVD",
            "YG214": "PVD",
            "YG3015": "CVD",
            "YG1010": "CVD",
            "YG401": "PVD",
            "YC0041": "cvd",
            "YG3115": "CVD",
            "YG2025": "CVD",
            "YT100": "NONE",
            "YG812": "X",
        }.get(grade, None)

    @staticmethod
    def grade_to_coating(grade: str | None) -> str | None:
        if grade is None:
            return None

        return {
            "YG812": "X",
        }.get(grade, None)
