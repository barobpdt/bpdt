# -*- coding: utf-8 -*-
"""
jamo=j2hcj(h2j(text))


allInit = []
text = "삼성전자"
for x in text:
	temp = h2j(x)
	imf = j2hcj(temp)  # init,middle,final
	print(f"{temp}, {imf}") # ㅅ, ㅅㅏㅁ
	allInit.append(imf[0])

print("".join(allInit)) 
"""

import os
from sys import stderr
from itertools import chain
import json
import re


_ROOT = os.path.abspath(os.path.dirname(__file__))

_JAMO_OFFSET = 44032
_JAMO_LEAD_OFFSET = 0x10ff
_JAMO_VOWEL_OFFSET = 0x1160
_JAMO_TAIL_OFFSET = 0x11a7

# U+11xx.json
{
  "\u1100": "HANGUL CHOSEONG KIYEOK",
  "\u1101": "HANGUL CHOSEONG SSANGKIYEOK",
  "\u1102": "HANGUL CHOSEONG NIEUN",
  "\u1103": "HANGUL CHOSEONG TIKEUT",
  "\u1104": "HANGUL CHOSEONG SSANGTIKEUT",
  "\u1105": "HANGUL CHOSEONG RIEUL",
  "\u1106": "HANGUL CHOSEONG MIEUM",
  "\u1107": "HANGUL CHOSEONG PIEUP",
  "\u1108": "HANGUL CHOSEONG SSANGPIEUP",
  "\u1109": "HANGUL CHOSEONG SIOS",
  "\u110a": "HANGUL CHOSEONG SSANGSIOS",
  "\u110b": "HANGUL CHOSEONG IEUNG",
  "\u110c": "HANGUL CHOSEONG CIEUC",
  "\u110d": "HANGUL CHOSEONG SSANGCIEUC",
  "\u110e": "HANGUL CHOSEONG CHIEUCH",
  "\u110f": "HANGUL CHOSEONG KHIEUKH",
  "\u1110": "HANGUL CHOSEONG THIEUTH",
  "\u1111": "HANGUL CHOSEONG PHIEUPH",
  "\u1112": "HANGUL CHOSEONG HIEUH",
  "\u1113": "HANGUL CHOSEONG NIEUN-KIYEOK",
  "\u1114": "HANGUL CHOSEONG SSANGNIEUN",
  "\u1115": "HANGUL CHOSEONG NIEUN-TIKEUT",
  "\u1116": "HANGUL CHOSEONG NIEUN-PIEUP",
  "\u1117": "HANGUL CHOSEONG TIKEUT-KIYEOK",
  "\u1118": "HANGUL CHOSEONG RIEUL-NIEUN",
  "\u1119": "HANGUL CHOSEONG SSANGRIEUL",
  "\u111a": "HANGUL CHOSEONG RIEUL-HIEUH",
  "\u111b": "HANGUL CHOSEONG KAPYEOUNRIEUL",
  "\u111c": "HANGUL CHOSEONG MIEUM-PIEUP",
  "\u111d": "HANGUL CHOSEONG KAPYEOUNMIEUM",
  "\u111e": "HANGUL CHOSEONG PIEUP-KIYEOK",
  "\u111f": "HANGUL CHOSEONG PIEUP-NIEUN",
  "\u1120": "HANGUL CHOSEONG PIEUP-TIKEUT",
  "\u1121": "HANGUL CHOSEONG PIEUP-SIOS",
  "\u1122": "HANGUL CHOSEONG PIEUP-SIOS-KIYEOK",
  "\u1123": "HANGUL CHOSEONG PIEUP-SIOS-TIKEUT",
  "\u1124": "HANGUL CHOSEONG PIEUP-SIOS-PIEUP",
  "\u1125": "HANGUL CHOSEONG PIEUP-SSANGSIOS",
  "\u1126": "HANGUL CHOSEONG PIEUP-SIOS-CIEUC",
  "\u1127": "HANGUL CHOSEONG PIEUP-CIEUC",
  "\u1128": "HANGUL CHOSEONG PIEUP-CHIEUCH",
  "\u1129": "HANGUL CHOSEONG PIEUP-THIEUTH",
  "\u112a": "HANGUL CHOSEONG PIEUP-PHIEUPH",
  "\u112b": "HANGUL CHOSEONG KAPYEOUNPIEUP",
  "\u112c": "HANGUL CHOSEONG KAPYEOUNSSANGPIEUP",
  "\u112d": "HANGUL CHOSEONG SIOS-KIYEOK",
  "\u112e": "HANGUL CHOSEONG SIOS-NIEUN",
  "\u112f": "HANGUL CHOSEONG SIOS-TIKEUT",
  "\u1130": "HANGUL CHOSEONG SIOS-RIEUL",
  "\u1131": "HANGUL CHOSEONG SIOS-MIEUM",
  "\u1132": "HANGUL CHOSEONG SIOS-PIEUP",
  "\u1133": "HANGUL CHOSEONG SIOS-PIEUP-KIYEOK",
  "\u1134": "HANGUL CHOSEONG SIOS-SSANGSIOS",
  "\u1135": "HANGUL CHOSEONG SIOS-IEUNG",
  "\u1136": "HANGUL CHOSEONG SIOS-CIEUC",
  "\u1137": "HANGUL CHOSEONG SIOS-CHIEUCH",
  "\u1138": "HANGUL CHOSEONG SIOS-KHIEUKH",
  "\u1139": "HANGUL CHOSEONG SIOS-THIEUTH",
  "\u113a": "HANGUL CHOSEONG SIOS-PHIEUPH",
  "\u113b": "HANGUL CHOSEONG SIOS-HIEUH",
  "\u113c": "HANGUL CHOSEONG CHITUEUMSIOS",
  "\u113d": "HANGUL CHOSEONG CHITUEUMSSANGSIOS",
  "\u113e": "HANGUL CHOSEONG CEONGCHIEUMSIOS",
  "\u113f": "HANGUL CHOSEONG CEONGCHIEUMSSANGSIOS",
  "\u1140": "HANGUL CHOSEONG PANSIOS",
  "\u1141": "HANGUL CHOSEONG IEUNG-KIYEOK",
  "\u1142": "HANGUL CHOSEONG IEUNG-TIKEUT",
  "\u1143": "HANGUL CHOSEONG IEUNG-MIEUM",
  "\u1144": "HANGUL CHOSEONG IEUNG-PIEUP",
  "\u1145": "HANGUL CHOSEONG IEUNG-SIOS",
  "\u1146": "HANGUL CHOSEONG IEUNG-PANSIOS",
  "\u1147": "HANGUL CHOSEONG SSANGIEUNG",
  "\u1148": "HANGUL CHOSEONG IEUNG-CIEUC",
  "\u1149": "HANGUL CHOSEONG IEUNG-CHIEUCH",
  "\u114a": "HANGUL CHOSEONG IEUNG-THIEUTH",
  "\u114b": "HANGUL CHOSEONG IEUNG-PHIEUPH",
  "\u114c": "HANGUL CHOSEONG YESIEUNG",
  "\u114d": "HANGUL CHOSEONG CIEUC-IEUNG",
  "\u114e": "HANGUL CHOSEONG CHITUEUMCIEUC",
  "\u114f": "HANGUL CHOSEONG CHITUEUMSSANGCIEUC",
  "\u1150": "HANGUL CHOSEONG CEONGCHIEUMCIEUC",
  "\u1151": "HANGUL CHOSEONG CEONGCHIEUMSSANGCIEUC",
  "\u1152": "HANGUL CHOSEONG CHIEUCH-KHIEUKH",
  "\u1153": "HANGUL CHOSEONG CHIEUCH-HIEUH",
  "\u1154": "HANGUL CHOSEONG CHITUEUMCHIEUCH",
  "\u1155": "HANGUL CHOSEONG CEONGCHIEUMCHIEUCH",
  "\u1156": "HANGUL CHOSEONG PHIEUPH-PIEUP",
  "\u1157": "HANGUL CHOSEONG KAPYEOUNPHIEUPH",
  "\u1158": "HANGUL CHOSEONG SSANGHIEUH",
  "\u1159": "HANGUL CHOSEONG YEORINHIEUH",
  "\u115a": "HANGUL CHOSEONG KIYEOK-TIKEUT",
  "\u115b": "HANGUL CHOSEONG NIEUN-SIOS",
  "\u115c": "HANGUL CHOSEONG NIEUN-CIEUC",
  "\u115d": "HANGUL CHOSEONG NIEUN-HIEUH",
  "\u115e": "HANGUL CHOSEONG TIKEUT-RIEUL",
  "\u115f": "HANGUL CHOSEONG FILLER",
  "\u1160": "HANGUL JUNGSEONG FILLER",
  "\u1161": "HANGUL JUNGSEONG A",
  "\u1162": "HANGUL JUNGSEONG AE",
  "\u1163": "HANGUL JUNGSEONG YA",
  "\u1164": "HANGUL JUNGSEONG YAE",
  "\u1165": "HANGUL JUNGSEONG EO",
  "\u1166": "HANGUL JUNGSEONG E",
  "\u1167": "HANGUL JUNGSEONG YEO",
  "\u1168": "HANGUL JUNGSEONG YE",
  "\u1169": "HANGUL JUNGSEONG O",
  "\u116a": "HANGUL JUNGSEONG WA",
  "\u116b": "HANGUL JUNGSEONG WAE",
  "\u116c": "HANGUL JUNGSEONG OE",
  "\u116d": "HANGUL JUNGSEONG YO",
  "\u116e": "HANGUL JUNGSEONG U",
  "\u116f": "HANGUL JUNGSEONG WEO",
  "\u1170": "HANGUL JUNGSEONG WE",
  "\u1171": "HANGUL JUNGSEONG WI",
  "\u1172": "HANGUL JUNGSEONG YU",
  "\u1173": "HANGUL JUNGSEONG EU",
  "\u1174": "HANGUL JUNGSEONG YI",
  "\u1175": "HANGUL JUNGSEONG I",
  "\u1176": "HANGUL JUNGSEONG A-O",
  "\u1177": "HANGUL JUNGSEONG A-U",
  "\u1178": "HANGUL JUNGSEONG YA-O",
  "\u1179": "HANGUL JUNGSEONG YA-YO",
  "\u117a": "HANGUL JUNGSEONG EO-O",
  "\u117b": "HANGUL JUNGSEONG EO-U",
  "\u117c": "HANGUL JUNGSEONG EO-EU",
  "\u117d": "HANGUL JUNGSEONG YEO-O",
  "\u117e": "HANGUL JUNGSEONG YEO-U",
  "\u117f": "HANGUL JUNGSEONG O-EO",
  "\u1180": "HANGUL JUNGSEONG O-E",
  "\u1181": "HANGUL JUNGSEONG O-YE",
  "\u1182": "HANGUL JUNGSEONG O-O",
  "\u1183": "HANGUL JUNGSEONG O-U",
  "\u1184": "HANGUL JUNGSEONG YO-YA",
  "\u1185": "HANGUL JUNGSEONG YO-YAE",
  "\u1186": "HANGUL JUNGSEONG YO-YEO",
  "\u1187": "HANGUL JUNGSEONG YO-O",
  "\u1188": "HANGUL JUNGSEONG YO-I",
  "\u1189": "HANGUL JUNGSEONG U-A",
  "\u118a": "HANGUL JUNGSEONG U-AE",
  "\u118b": "HANGUL JUNGSEONG U-EO-EU",
  "\u118c": "HANGUL JUNGSEONG U-YE",
  "\u118d": "HANGUL JUNGSEONG U-U",
  "\u118e": "HANGUL JUNGSEONG YU-A",
  "\u118f": "HANGUL JUNGSEONG YU-EO",
  "\u1190": "HANGUL JUNGSEONG YU-E",
  "\u1191": "HANGUL JUNGSEONG YU-YEO",
  "\u1192": "HANGUL JUNGSEONG YU-YE",
  "\u1193": "HANGUL JUNGSEONG YU-U",
  "\u1194": "HANGUL JUNGSEONG YU-I",
  "\u1195": "HANGUL JUNGSEONG EU-U",
  "\u1196": "HANGUL JUNGSEONG EU-EU",
  "\u1197": "HANGUL JUNGSEONG YI-U",
  "\u1198": "HANGUL JUNGSEONG I-A",
  "\u1199": "HANGUL JUNGSEONG I-YA",
  "\u119a": "HANGUL JUNGSEONG I-O",
  "\u119b": "HANGUL JUNGSEONG I-U",
  "\u119c": "HANGUL JUNGSEONG I-EU",
  "\u119d": "HANGUL JUNGSEONG I-ARAEA",
  "\u119e": "HANGUL JUNGSEONG ARAEA",
  "\u119f": "HANGUL JUNGSEONG ARAEA-EO",
  "\u11a0": "HANGUL JUNGSEONG ARAEA-U",
  "\u11a1": "HANGUL JUNGSEONG ARAEA-I",
  "\u11a2": "HANGUL JUNGSEONG SSANGARAEA",
  "\u11a3": "HANGUL JUNGSEONG A-EU",
  "\u11a4": "HANGUL JUNGSEONG YA-U",
  "\u11a5": "HANGUL JUNGSEONG YEO-YA",
  "\u11a6": "HANGUL JUNGSEONG O-YA",
  "\u11a7": "HANGUL JUNGSEONG O-YAE",
  "\u11a8": "HANGUL JONGSEONG KIYEOK",
  "\u11a9": "HANGUL JONGSEONG SSANGKIYEOK",
  "\u11aa": "HANGUL JONGSEONG KIYEOK-SIOS",
  "\u11ab": "HANGUL JONGSEONG NIEUN",
  "\u11ac": "HANGUL JONGSEONG NIEUN-CIEUC",
  "\u11ad": "HANGUL JONGSEONG NIEUN-HIEUH",
  "\u11ae": "HANGUL JONGSEONG TIKEUT",
  "\u11af": "HANGUL JONGSEONG RIEUL",
  "\u11b0": "HANGUL JONGSEONG RIEUL-KIYEOK",
  "\u11b1": "HANGUL JONGSEONG RIEUL-MIEUM",
  "\u11b2": "HANGUL JONGSEONG RIEUL-PIEUP",
  "\u11b3": "HANGUL JONGSEONG RIEUL-SIOS",
  "\u11b4": "HANGUL JONGSEONG RIEUL-THIEUTH",
  "\u11b5": "HANGUL JONGSEONG RIEUL-PHIEUPH",
  "\u11b6": "HANGUL JONGSEONG RIEUL-HIEUH",
  "\u11b7": "HANGUL JONGSEONG MIEUM",
  "\u11b8": "HANGUL JONGSEONG PIEUP",
  "\u11b9": "HANGUL JONGSEONG PIEUP-SIOS",
  "\u11ba": "HANGUL JONGSEONG SIOS",
  "\u11bb": "HANGUL JONGSEONG SSANGSIOS",
  "\u11bc": "HANGUL JONGSEONG IEUNG",
  "\u11bd": "HANGUL JONGSEONG CIEUC",
  "\u11be": "HANGUL JONGSEONG CHIEUCH",
  "\u11bf": "HANGUL JONGSEONG KHIEUKH",
  "\u11c0": "HANGUL JONGSEONG THIEUTH",
  "\u11c1": "HANGUL JONGSEONG PHIEUPH",
  "\u11c2": "HANGUL JONGSEONG HIEUH",
  "\u11c3": "HANGUL JONGSEONG KIYEOK-RIEUL",
  "\u11c4": "HANGUL JONGSEONG KIYEOK-SIOS-KIYEOK",
  "\u11c5": "HANGUL JONGSEONG NIEUN-KIYEOK",
  "\u11c6": "HANGUL JONGSEONG NIEUN-TIKEUT",
  "\u11c7": "HANGUL JONGSEONG NIEUN-SIOS",
  "\u11c8": "HANGUL JONGSEONG NIEUN-PANSIOS",
  "\u11c9": "HANGUL JONGSEONG NIEUN-THIEUTH",
  "\u11ca": "HANGUL JONGSEONG TIKEUT-KIYEOK",
  "\u11cb": "HANGUL JONGSEONG TIKEUT-RIEUL",
  "\u11cc": "HANGUL JONGSEONG RIEUL-KIYEOK-SIOS",
  "\u11cd": "HANGUL JONGSEONG RIEUL-NIEUN",
  "\u11ce": "HANGUL JONGSEONG RIEUL-TIKEUT",
  "\u11cf": "HANGUL JONGSEONG RIEUL-TIKEUT-HIEUH",
  "\u11d0": "HANGUL JONGSEONG SSANGRIEUL",
  "\u11d1": "HANGUL JONGSEONG RIEUL-MIEUM-KIYEOK",
  "\u11d2": "HANGUL JONGSEONG RIEUL-MIEUM-SIOS",
  "\u11d3": "HANGUL JONGSEONG RIEUL-PIEUP-SIOS",
  "\u11d4": "HANGUL JONGSEONG RIEUL-PIEUP-HIEUH",
  "\u11d5": "HANGUL JONGSEONG RIEUL-KAPYEOUNPIEUP",
  "\u11d6": "HANGUL JONGSEONG RIEUL-SSANGSIOS",
  "\u11d7": "HANGUL JONGSEONG RIEUL-PANSIOS",
  "\u11d8": "HANGUL JONGSEONG RIEUL-KHIEUKH",
  "\u11d9": "HANGUL JONGSEONG RIEUL-YEORINHIEUH",
  "\u11da": "HANGUL JONGSEONG MIEUM-KIYEOK",
  "\u11db": "HANGUL JONGSEONG MIEUM-RIEUL",
  "\u11dc": "HANGUL JONGSEONG MIEUM-PIEUP",
  "\u11dd": "HANGUL JONGSEONG MIEUM-SIOS",
  "\u11de": "HANGUL JONGSEONG MIEUM-SSANGSIOS",
  "\u11df": "HANGUL JONGSEONG MIEUM-PANSIOS",
  "\u11e0": "HANGUL JONGSEONG MIEUM-CHIEUCH",
  "\u11e1": "HANGUL JONGSEONG MIEUM-HIEUH",
  "\u11e2": "HANGUL JONGSEONG KAPYEOUNMIEUM",
  "\u11e3": "HANGUL JONGSEONG PIEUP-RIEUL",
  "\u11e4": "HANGUL JONGSEONG PIEUP-PHIEUPH",
  "\u11e5": "HANGUL JONGSEONG PIEUP-HIEUH",
  "\u11e6": "HANGUL JONGSEONG KAPYEOUNPIEUP",
  "\u11e7": "HANGUL JONGSEONG SIOS-KIYEOK",
  "\u11e8": "HANGUL JONGSEONG SIOS-TIKEUT",
  "\u11e9": "HANGUL JONGSEONG SIOS-RIEUL",
  "\u11ea": "HANGUL JONGSEONG SIOS-PIEUP",
  "\u11eb": "HANGUL JONGSEONG PANSIOS",
  "\u11ec": "HANGUL JONGSEONG IEUNG-KIYEOK",
  "\u11ed": "HANGUL JONGSEONG IEUNG-SSANGKIYEOK",
  "\u11ee": "HANGUL JONGSEONG SSANGIEUNG",
  "\u11ef": "HANGUL JONGSEONG IEUNG-KHIEUKH",
  "\u11f0": "HANGUL JONGSEONG YESIEUNG",
  "\u11f1": "HANGUL JONGSEONG YESIEUNG-SIOS",
  "\u11f2": "HANGUL JONGSEONG YESIEUNG-PANSIOS",
  "\u11f3": "HANGUL JONGSEONG PHIEUPH-PIEUP",
  "\u11f4": "HANGUL JONGSEONG KAPYEOUNPHIEUPH",
  "\u11f5": "HANGUL JONGSEONG HIEUH-NIEUN",
  "\u11f6": "HANGUL JONGSEONG HIEUH-RIEUL",
  "\u11f7": "HANGUL JONGSEONG HIEUH-MIEUM",
  "\u11f8": "HANGUL JONGSEONG HIEUH-PIEUP",
  "\u11f9": "HANGUL JONGSEONG YEORINHIEUH",
  "\u11fa": "HANGUL JONGSEONG KIYEOK-NIEUN",
  "\u11fb": "HANGUL JONGSEONG KIYEOK-PIEUP",
  "\u11fc": "HANGUL JONGSEONG KIYEOK-CHIEUCH",
  "\u11fd": "HANGUL JONGSEONG KIYEOK-KHIEUKH",
  "\u11fe": "HANGUL JONGSEONG KIYEOK-HIEUH",
  "\u11ff": "HANGUL JONGSEONG SSANGNIEUN"
}

with open(os.path.join(_ROOT, 'data', "U+11xx.json"), 'r') as namedata:
	_JAMO_TO_NAME = json.load(namedata)
_JAMO_REVERSE_LOOKUP = {name: char for char, name in _JAMO_TO_NAME.items()}

# U+31xx.json
{
  "\u3131": "HANGUL LETTER KIYEOK",
  "\u3132": "HANGUL LETTER SSANGKIYEOK",
  "\u3133": "HANGUL LETTER KIYEOK-SIOS",
  "\u3134": "HANGUL LETTER NIEUN",
  "\u3135": "HANGUL LETTER NIEUN-CIEUC",
  "\u3136": "HANGUL LETTER NIEUN-HIEUH",
  "\u3137": "HANGUL LETTER TIKEUT",
  "\u3138": "HANGUL LETTER SSANGTIKEUT",
  "\u3139": "HANGUL LETTER RIEUL",
  "\u313a": "HANGUL LETTER RIEUL-KIYEOK",
  "\u313b": "HANGUL LETTER RIEUL-MIEUM",
  "\u313c": "HANGUL LETTER RIEUL-PIEUP",
  "\u313d": "HANGUL LETTER RIEUL-SIOS",
  "\u313e": "HANGUL LETTER RIEUL-THIEUTH",
  "\u313f": "HANGUL LETTER RIEUL-PHIEUPH",
  "\u3140": "HANGUL LETTER RIEUL-HIEUH",
  "\u3141": "HANGUL LETTER MIEUM",
  "\u3142": "HANGUL LETTER PIEUP",
  "\u3143": "HANGUL LETTER SSANGPIEUP",
  "\u3144": "HANGUL LETTER PIEUP-SIOS",
  "\u3145": "HANGUL LETTER SIOS",
  "\u3146": "HANGUL LETTER SSANGSIOS",
  "\u3147": "HANGUL LETTER IEUNG",
  "\u3148": "HANGUL LETTER CIEUC",
  "\u3149": "HANGUL LETTER SSANGCIEUC",
  "\u314a": "HANGUL LETTER CHIEUCH",
  "\u314b": "HANGUL LETTER KHIEUKH",
  "\u314c": "HANGUL LETTER THIEUTH",
  "\u314d": "HANGUL LETTER PHIEUPH",
  "\u314e": "HANGUL LETTER HIEUH",
  "\u314f": "HANGUL LETTER A",
  "\u3150": "HANGUL LETTER AE",
  "\u3151": "HANGUL LETTER YA",
  "\u3152": "HANGUL LETTER YAE",
  "\u3153": "HANGUL LETTER EO",
  "\u3154": "HANGUL LETTER E",
  "\u3155": "HANGUL LETTER YEO",
  "\u3156": "HANGUL LETTER YE",
  "\u3157": "HANGUL LETTER O",
  "\u3158": "HANGUL LETTER WA",
  "\u3159": "HANGUL LETTER WAE",
  "\u315a": "HANGUL LETTER OE",
  "\u315b": "HANGUL LETTER YO",
  "\u315c": "HANGUL LETTER U",
  "\u315d": "HANGUL LETTER WEO",
  "\u315e": "HANGUL LETTER WE",
  "\u315f": "HANGUL LETTER WI",
  "\u3160": "HANGUL LETTER YU",
  "\u3161": "HANGUL LETTER EU",
  "\u3162": "HANGUL LETTER YI",
  "\u3163": "HANGUL LETTER I",
  "\u3164": "HANGUL FILLER",
  "\u3165": "HANGUL LETTER SSANGNIEUN",
  "\u3166": "HANGUL LETTER NIEUN-TIKEUT",
  "\u3167": "HANGUL LETTER NIEUN-SIOS",
  "\u3168": "HANGUL LETTER NIEUN-PANSIOS",
  "\u3169": "HANGUL LETTER RIEUL-KIYEOK-SIOS",
  "\u316a": "HANGUL LETTER RIEUL-TIKEUT",
  "\u316b": "HANGUL LETTER RIEUL-PIEUP-SIOS",
  "\u316c": "HANGUL LETTER RIEUL-PANSIOS",
  "\u316d": "HANGUL LETTER RIEUL-YEORINHIEUH",
  "\u316e": "HANGUL LETTER MIEUM-PIEUP",
  "\u316f": "HANGUL LETTER MIEUM-SIOS",
  "\u3170": "HANGUL LETTER MIEUM-PANSIOS",
  "\u3171": "HANGUL LETTER KAPYEOUNMIEUM",
  "\u3172": "HANGUL LETTER PIEUP-KIYEOK",
  "\u3173": "HANGUL LETTER PIEUP-TIKEUT",
  "\u3174": "HANGUL LETTER PIEUP-SIOS-KIYEOK",
  "\u3175": "HANGUL LETTER PIEUP-SIOS-TIKEUT",
  "\u3176": "HANGUL LETTER PIEUP-CIEUC",
  "\u3177": "HANGUL LETTER PIEUP-THIEUTH",
  "\u3178": "HANGUL LETTER KAPYEOUNPIEUP",
  "\u3179": "HANGUL LETTER KAPYEOUNSSANGPIEUP",
  "\u317a": "HANGUL LETTER SIOS-KIYEOK",
  "\u317b": "HANGUL LETTER SIOS-NIEUN",
  "\u317c": "HANGUL LETTER SIOS-TIKEUT",
  "\u317d": "HANGUL LETTER SIOS-PIEUP",
  "\u317e": "HANGUL LETTER SIOS-CIEUC",
  "\u317f": "HANGUL LETTER PANSIOS",
  "\u3180": "HANGUL LETTER SSANGIEUNG",
  "\u3181": "HANGUL LETTER YESIEUNG",
  "\u3182": "HANGUL LETTER YESIEUNG-SIOS",
  "\u3183": "HANGUL LETTER YESIEUNG-PANSIOS",
  "\u3184": "HANGUL LETTER KAPYEOUNPHIEUPH",
  "\u3185": "HANGUL LETTER SSANGHIEUH",
  "\u3186": "HANGUL LETTER YEORINHIEUH",
  "\u3187": "HANGUL LETTER YO-YA",
  "\u3188": "HANGUL LETTER YO-YAE",
  "\u3189": "HANGUL LETTER YO-I",
  "\u318a": "HANGUL LETTER YU-YEO",
  "\u318b": "HANGUL LETTER YU-YE",
  "\u318c": "HANGUL LETTER YU-I",
  "\u318d": "HANGUL LETTER ARAEA",
  "\u318e": "HANGUL LETTER ARAEAE"
}
with open(os.path.join(_ROOT, 'data', "U+31xx.json"), 'r') as namedata:
	_HCJ_TO_NAME = json.load(namedata)
_HCJ_REVERSE_LOOKUP = {name: char for char, name in _HCJ_TO_NAME.items()}

# decompositions.json
{
  "\u1101": ["\u1100", "\u1100"],
  "\u1104": ["\u1103", "\u1103"],
  "\u1108": ["\u1107", "\u1107"],
  "\u110a": ["\u1109", "\u1109"],
  "\u110d": ["\u110c", "\u110c"],
  "\u1113": ["\u1102", "\u1100"],
  "\u1114": ["\u1102", "\u1102"],
  "\u1115": ["\u1102", "\u1103"],
  "\u1116": ["\u1102", "\u1107"],
  "\u1117": ["\u1103", "\u1100"],
  "\u1118": ["\u1105", "\u1102"],
  "\u1119": ["\u1105", "\u1105"],
  "\u111a": ["\u1105", "\u1112"],
  "\u111b": ["\u1105", "\u110b"],
  "\u111c": ["\u1106", "\u1107"],
  "\u111d": ["\u1106", "\u110b"],
  "\u111e": ["\u1107", "\u1100"],
  "\u111f": ["\u1107", "\u1102"],
  "\u1120": ["\u1107", "\u1103"],
  "\u1121": ["\u1107", "\u1109"],
  "\u1122": ["\u1107", "\u1109", "\u1100"],
  "\u1123": ["\u1107", "\u1109", "\u1103"],
  "\u1124": ["\u1107", "\u1109", "\u1107"],
  "\u1125": ["\u1107", "\u1109", "\u1109"],
  "\u1126": ["\u1107", "\u1109", "\u110c"],
  "\u1127": ["\u1107", "\u110c"],
  "\u1128": ["\u1107", "\u110e"],
  "\u1129": ["\u1107", "\u1110"],
  "\u112a": ["\u1107", "\u1111"],
  "\u112b": ["\u1107", "\u110b"],
  "\u112c": ["\u1107", "\u1107", "\u110b"],
  "\u112d": ["\u1109", "\u1100"],
  "\u112e": ["\u1109", "\u1102"],
  "\u112f": ["\u1109", "\u1103"],
  "\u1130": ["\u1109", "\u1105"],
  "\u1131": ["\u1109", "\u1106"],
  "\u1132": ["\u1109", "\u1107"],
  "\u1133": ["\u1109", "\u1107", "\u1100"],
  "\u1134": ["\u1109", "\u1109", "\u1109"],
  "\u1135": ["\u1109", "\u114c"],
  "\u1136": ["\u1109", "\u110c"],
  "\u1137": ["\u1109", "\u110e"],
  "\u1138": ["\u1109", "\u110f"],
  "\u1139": ["\u1109", "\u1110"],
  "\u113a": ["\u1109", "\u1111"],
  "\u113b": ["\u1109", "\u1112"],
  "\u113d": ["\u113c", "\u113c"],
  "\u113f": ["\u113e", "\u113e"],
  "\u1141": ["\u114c", "\u1100"],
  "\u1142": ["\u114c", "\u1103"],
  "\u1143": ["\u114c", "\u1106"],
  "\u1144": ["\u114c", "\u1107"],
  "\u1145": ["\u114c", "\u1109"],
  "\u1146": ["\u114c", "\u1140"],
  "\u1147": ["\u110b", "\u110b"],
  "\u1148": ["\u114c", "\u110c"],
  "\u1149": ["\u114c", "\u110e"],
  "\u114a": ["\u114c", "\u1110"],
  "\u114b": ["\u114c", "\u1111"],
  "\u114d": ["\u110c", "\u114c"],
  "\u114f": ["\u114e", "\u114e"],
  "\u1151": ["\u1150", "\u1150"],
  "\u1152": ["\u110e", "\u110f"],
  "\u1153": ["\u110e", "\u1112"],
  "\u1156": ["\u1111", "\u1107"],
  "\u1157": ["\u1111", "\u110b"],
  "\u1158": ["\u1112", "\u1112"],
  "\u115a": ["\u1100", "\u1103"],
  "\u115b": ["\u1102", "\u1109"],
  "\u115c": ["\u1102", "\u110c"],
  "\u115d": ["\u1102", "\u1112"],
  "\u115e": ["\u1103", "\u1105"],
  "\u1162": ["\u1161", "\u1175"],
  "\u1164": ["\u1163", "\u1175"],
  "\u1166": ["\u1165", "\u1175"],
  "\u1168": ["\u1167", "\u1175"],
  "\u116a": ["\u1169", "\u1161"],
  "\u116b": ["\u1169", "\u1161", "\u1175"],
  "\u116c": ["\u1169", "\u1175"],
  "\u116f": ["\u116e", "\u1165"],
  "\u1170": ["\u116e", "\u1165", "\u1175"],
  "\u1171": ["\u116e", "\u1175"],
  "\u1174": ["\u1173", "\u1175"],
  "\u1176": ["\u1161", "\u1169"],
  "\u1177": ["\u1161", "\u116e"],
  "\u1178": ["\u1163", "\u1169"],
  "\u1179": ["\u1163", "\u116d"],
  "\u117a": ["\u1165", "\u1169"],
  "\u117b": ["\u1165", "\u116e"],
  "\u117c": ["\u1165", "\u1173"],
  "\u117d": ["\u1167", "\u1169"],
  "\u117e": ["\u1167", "\u116e"],
  "\u117f": ["\u1169", "\u1165"],
  "\u1180": ["\u1169", "\u1165", "\u1175"],
  "\u1181": ["\u1169", "\u1167", "\u1175"],
  "\u1182": ["\u1169", "\u1169"],
  "\u1183": ["\u1169", "\u116e"],
  "\u1184": ["\u116d", "\u1163"],
  "\u1185": ["\u116d", "\u1163", "\u1175"],
  "\u1186": ["\u116d", "\u1167"],
  "\u1187": ["\u116d", "\u1169"],
  "\u1188": ["\u116d", "\u1175"],
  "\u1189": ["\u116e", "\u1161"],
  "\u118a": ["\u116e", "\u1161", "\u1175"],
  "\u118b": ["\u116e", "\u1165", "\u1173"],
  "\u118c": ["\u116e", "\u1167", "\u1175"],
  "\u118d": ["\u116e", "\u116e"],
  "\u118e": ["\u1172", "\u1161"],
  "\u118f": ["\u1172", "\u1165"],
  "\u1190": ["\u1172", "\u1165", "\u1175"],
  "\u1191": ["\u1172", "\u1167"],
  "\u1192": ["\u1172", "\u1167", "\u1175"],
  "\u1193": ["\u1172", "\u116e"],
  "\u1194": ["\u1172", "\u1175"],
  "\u1195": ["\u1173", "\u116e"],
  "\u1196": ["\u1173", "\u1173"],
  "\u1197": ["\u1173", "\u1175", "\u116e"],
  "\u1198": ["\u1175", "\u1161"],
  "\u1199": ["\u1175", "\u1163"],
  "\u119a": ["\u1175", "\u1169"],
  "\u119b": ["\u1175", "\u116e"],
  "\u119c": ["\u1175", "\u1173"],
  "\u119d": ["\u1175", "\u119e"],
  "\u119f": ["\u119e", "\u1165"],
  "\u11a0": ["\u119e", "\u116e"],
  "\u11a1": ["\u119e", "\u1175"],
  "\u11a2": ["\u119e", "\u119e"],
  "\u11a3": ["\u1161", "\u1173"],
  "\u11a4": ["\u1163", "\u116e"],
  "\u11a5": ["\u1167", "\u1163"],
  "\u11a6": ["\u1169", "\u1163"],
  "\u11a7": ["\u1169", "\u1164"],
  "\u11a9": ["\u11a8", "\u11a8"],
  "\u11aa": ["\u11a8", "\u11ba"],
  "\u11ac": ["\u11ab", "\u11bd"],
  "\u11ad": ["\u11ab", "\u11c2"],
  "\u11b0": ["\u11af", "\u11a8"],
  "\u11b1": ["\u11af", "\u11b7"],
  "\u11b2": ["\u11af", "\u11b8"],
  "\u11b3": ["\u11af", "\u11ba"],
  "\u11b4": ["\u11af", "\u11c0"],
  "\u11b5": ["\u11af", "\u11c1"],
  "\u11b6": ["\u11af", "\u11c2"],
  "\u11b9": ["\u11b8", "\u11ba"],
  "\u11bb": ["\u11ba", "\u11ba"],
  "\u11c3": ["\u11a8", "\u11af"],
  "\u11c4": ["\u11a8", "\u11ba", "\u11a8"],
  "\u11c5": ["\u11ab", "\u11a8"],
  "\u11c6": ["\u11ab", "\u11ae"],
  "\u11c7": ["\u11ab", "\u11ba"],
  "\u11c8": ["\u11ab", "\u11eb"],
  "\u11c9": ["\u11ab", "\u11c0"],
  "\u11ca": ["\u11ae", "\u11a8"],
  "\u11cb": ["\u11ae", "\u11af"],
  "\u11cc": ["\u11af", "\u11a8", "\u11ba"],
  "\u11cd": ["\u11af", "\u11ab"],
  "\u11ce": ["\u11af", "\u11ae"],
  "\u11cf": ["\u11af", "\u11ae", "\u11c2"],
  "\u11d0": ["\u11af", "\u11af"],
  "\u11d1": ["\u11af", "\u11b7", "\u11a8"],
  "\u11d2": ["\u11af", "\u11b7", "\u11ba"],
  "\u11d3": ["\u11af", "\u11b8", "\u11ba"],
  "\u11d4": ["\u11af", "\u11b8", "\u11c2"],
  "\u11d5": ["\u11af", "\u11b8", "\u11bc"],
  "\u11d6": ["\u11af", "\u11ba", "\u11ba"],
  "\u11d7": ["\u11af", "\u11eb"],
  "\u11d8": ["\u11af", "\u11bf"],
  "\u11d9": ["\u11af", "\u11f9"],
  "\u11da": ["\u11b7", "\u11a8"],
  "\u11db": ["\u11b7", "\u11af"],
  "\u11dc": ["\u11b7", "\u11b8"],
  "\u11dd": ["\u11b7", "\u11ba"],
  "\u11de": ["\u11b7", "\u11ba", "\u11ba"],
  "\u11df": ["\u11b7", "\u11eb"],
  "\u11e0": ["\u11b7", "\u11be"],
  "\u11e1": ["\u11b7", "\u11c2"],
  "\u11e2": ["\u11b7", "\u11bc"],
  "\u11e3": ["\u11b8", "\u11af"],
  "\u11e4": ["\u11b8", "\u11c1"],
  "\u11e5": ["\u11b8", "\u11c2"],
  "\u11e6": ["\u11b8", "\u11bc"],
  "\u11e7": ["\u11ba", "\u11a8"],
  "\u11e8": ["\u11ba", "\u11ae"],
  "\u11e9": ["\u11ba", "\u11af"],
  "\u11ea": ["\u11ba", "\u11b8"],
  "\u11ec": ["\u11f0", "\u11a8"],
  "\u11ed": ["\u11f0", "\u11a8", "\u11a8"],
  "\u11ee": ["\u11f0", "\u11f0"],
  "\u11ef": ["\u11f0", "\u11bf"],
  "\u11f1": ["\u11f0", "\u11ba"],
  "\u11f2": ["\u11f0", "\u11eb"],
  "\u11f3": ["\u11c1", "\u11b8"],
  "\u11f4": ["\u11c1", "\u11bc"],
  "\u11f5": ["\u11c2", "\u11ab"],
  "\u11f6": ["\u11c2", "\u11af"],
  "\u11f7": ["\u11c2", "\u11b7"],
  "\u11f8": ["\u11c2", "\u11b8"],
  "\u11fa": ["\u11a8", "\u11ab"],
  "\u11fb": ["\u11a8", "\u11b8"],
  "\u11fc": ["\u11a8", "\u11be"],
  "\u11fd": ["\u11a8", "\u11bf"],
  "\u11fe": ["\u11a8", "\u11c2"],
  "\u11ff": ["\u11ab", "\u11ab"],
  "\u3132": ["\u3131", "\u3131"],
  "\u3133": ["\u3131", "\u3145"],
  "\u3135": ["\u3134", "\u3148"],
  "\u3136": ["\u3134", "\u314e"],
  "\u3138": ["\u3137", "\u3137"],
  "\u313a": ["\u3139", "\u3131"],
  "\u313b": ["\u3139", "\u3141"],
  "\u313c": ["\u3139", "\u3142"],
  "\u313d": ["\u3139", "\u3145"],
  "\u313e": ["\u3139", "\u314c"],
  "\u313f": ["\u3139", "\u314d"],
  "\u3140": ["\u3139", "\u314e"],
  "\u3143": ["\u3142", "\u3142"],
  "\u3144": ["\u3142", "\u3145"],
  "\u3146": ["\u3145", "\u3145"],
  "\u3149": ["\u3148", "\u3148"],
  "\u3150": ["\u314f", "\u3163"],
  "\u3152": ["\u3151", "\u3163"],
  "\u3154": ["\u3153", "\u3163"],
  "\u3156": ["\u3155", "\u3163"],
  "\u3158": ["\u3157", "\u314f"],
  "\u3159": ["\u3157", "\u314f", "\u3163"],
  "\u315a": ["\u3157", "\u3163"],
  "\u315d": ["\u315c", "\u3153"],
  "\u315e": ["\u315c", "\u3153", "\u3163"],
  "\u315f": ["\u315c", "\u3163"],
  "\u3162": ["\u3161", "\u3163"],
  "\u3165": ["\u3134", "\u3134"],
  "\u3166": ["\u3134", "\u3137"],
  "\u3167": ["\u3134", "\u3145"],
  "\u3168": ["\u3134", "\u317f"],
  "\u3169": ["\u3139", "\u3131", "\u3145"],
  "\u316a": ["\u3139", "\u3137"],
  "\u316b": ["\u3139", "\u3142", "\u3145"],
  "\u316c": ["\u3139", "\u317f"],
  "\u316d": ["\u3139", "\u3186"],
  "\u316e": ["\u3141", "\u3142"],
  "\u316f": ["\u3141", "\u3145"],
  "\u3170": ["\u3141", "\u317f"],
  "\u3171": ["\u3141", "\u3147"],
  "\u3172": ["\u3142", "\u3131"],
  "\u3173": ["\u3142", "\u3137"],
  "\u3174": ["\u3142", "\u3145", "\u3131"],
  "\u3175": ["\u3142", "\u3145", "\u3137"],
  "\u3176": ["\u3142", "\u3148"],
  "\u3177": ["\u3142", "\u314c"],
  "\u3178": ["\u3142", "\u3147"],
  "\u3179": ["\u3142", "\u3142", "\u3147"],
  "\u317a": ["\u3145", "\u3131"],
  "\u317b": ["\u3145", "\u3134"],
  "\u317c": ["\u3145", "\u3137"],
  "\u317d": ["\u3145", "\u3142"],
  "\u317e": ["\u3145", "\u3148"],
  "\u3180": ["\u3147", "\u3147"],
  "\u3182": ["\u3181", "\u3145"],
  "\u3183": ["\u3181", "\u317f"],
  "\u3184": ["\u314d", "\u3147"],
  "\u3185": ["\u314e", "\u314e"],
  "\u3187": ["\u315b", "\u3151"],
  "\u3188": ["\u315b", "\u3151", "\u3163"],
  "\u3189": ["\u315b", "\u3163"],
  "\u318a": ["\u3160", "\u3155"],
  "\u318b": ["\u3160", "\u3155", "\u3163"],
  "\u318c": ["\u3160", "\u3163"],
  "\u318e": ["\u318d", "\u3163"],
  "\ua960": ["\u1103", "\u1106"],
  "\ua961": ["\u1103", "\u1107"],
  "\ua962": ["\u1103", "\u1109"],
  "\ua963": ["\u1103", "\u110c"],
  "\ua964": ["\u1105", "\u1100"],
  "\ua965": ["\u1105", "\u1100", "\u1100"],
  "\ua966": ["\u1105", "\u1103"],
  "\ua967": ["\u1105", "\u1103", "\u1103"],
  "\ua968": ["\u1105", "\u1106"],
  "\ua969": ["\u1105", "\u1107"],
  "\ua96a": ["\u1105", "\u1107", "\u1107"],
  "\ua96b": ["\u1105", "\u112b"],
  "\ua96c": ["\u1105", "\u1109"],
  "\ua96d": ["\u1105", "\u110c"],
  "\ua96e": ["\u1105", "\u110f"],
  "\ua96f": ["\u1106", "\u1100"],
  "\ua970": ["\u1106", "\u1103"],
  "\ua971": ["\u1106", "\u1109"],
  "\ua972": ["\u1107", "\u1109", "\u1110"],
  "\ua973": ["\u1107", "\u110f"],
  "\ua974": ["\u1107", "\u1112"],
  "\ua975": ["\u1109", "\u1109", "\u1107"],
  "\ua976": ["\u110b", "\u1105"],
  "\ua977": ["\u110b", "\u1112"],
  "\ua978": ["\u110c", "\u110c", "\u1112"],
  "\ua979": ["\u1110", "\u1110"],
  "\ua97a": ["\u1111", "\u1112"],
  "\ua97b": ["\u1112", "\u1109"],
  "\ua97c": ["\u1159", "\u1159"],
  "\ud7b0": ["\u1169", "\u1167"],
  "\ud7b1": ["\u1169", "\u1169", "\u1175"],
  "\ud7b2": ["\u116d", "\u1161"],
  "\ud7b3": ["\u116d", "\u1162"],
  "\ud7b4": ["\u116d", "\u1165"],
  "\ud7b5": ["\u116e", "\u1167"],
  "\ud7b6": ["\u116e", "\u1175", "\u1175"],
  "\ud7b7": ["\u1172", "\u1162"],
  "\ud7b8": ["\u1172", "\u1169"],
  "\ud7b9": ["\u1173", "\u1161"],
  "\ud7ba": ["\u1173", "\u1165"],
  "\ud7bb": ["\u1173", "\u1166"],
  "\ud7bc": ["\u1173", "\u1169"],
  "\ud7bd": ["\u1175", "\u1163", "\u1169"],
  "\ud7be": ["\u1175", "\u1164"],
  "\ud7bf": ["\u1175", "\u1167"],
  "\ud7c0": ["\u1175", "\u1168"],
  "\ud7c1": ["\u1175", "\u1169", "\u1175"],
  "\ud7c2": ["\u1175", "\u116d"],
  "\ud7c3": ["\u1175", "\u1172"],
  "\ud7c4": ["\u1175", "\u1175"],
  "\ud7c5": ["\u119e", "\u1161"],
  "\ud7c6": ["\u119e", "\u1166"],
  "\ud7cb": ["\u11ab", "\u11af"],
  "\ud7cc": ["\u11ab", "\u11be"],
  "\ud7cd": ["\u11ae", "\u11ae"],
  "\ud7ce": ["\u11b8", "\u11ae", "\u11ae"],
  "\ud7cf": ["\u11ae", "\u11b8"],
  "\ud7d0": ["\u11ae", "\u11ba"],
  "\ud7d1": ["\u11ae", "\u11ba", "\u11a8"],
  "\ud7d2": ["\u11ae", "\u11bd"],
  "\ud7d3": ["\u11ae", "\u11be"],
  "\ud7d4": ["\u11ae", "\u11c0"],
  "\ud7d5": ["\u11af", "\u11a8", "\u11a8"],
  "\ud7d6": ["\u11af", "\u11a8", "\u11c2"],
  "\ud7d7": ["\u11af", "\u11af", "\u11bf"],
  "\ud7d8": ["\u11af", "\u11b7", "\u11c2"],
  "\ud7d9": ["\u11af", "\u11b8", "\u11ae"],
  "\ud7da": ["\u11af", "\u11b8", "\u11c1"],
  "\ud7db": ["\u11af", "\u11f0"],
  "\ud7dc": ["\u11af", "\u11f9", "\u11c2"],
  "\ud7de": ["\u11b7", "\u11ab"],
  "\ud7df": ["\u11b7", "\u11ab", "\u11ab"],
  "\ud7e0": ["\u11b7", "\u11b7"],
  "\ud7e1": ["\u11b7", "\u11b8", "\u11ba"],
  "\ud7e2": ["\u11b7", "\u11bd"],
  "\ud7e3": ["\u11b8", "\u11ae"],
  "\ud7e4": ["\u11b8", "\u11af", "\u11c1"],
  "\ud7e5": ["\u11b8", "\u11b7"],
  "\ud7e6": ["\u11b8", "\u11b8"],
  "\ud7e7": ["\u11b8", "\u11ba", "\u11ae"],
  "\ud7e8": ["\u11b8", "\u11bd"],
  "\ud7e9": ["\u11b8", "\u11be"],
  "\ud7ea": ["\u11ba", "\u11b7"],
  "\ud7eb": ["\u11ba", "\u11b8", "\u11bc"],
  "\ud7ec": ["\u11ba", "\u11ba", "\u11a8"],
  "\ud7ed": ["\u11ba", "\u11ba", "\u11ae"],
  "\ud7ee": ["\u11ba", "\u11eb"],
  "\ud7ef": ["\u11ba", "\u11bd"],
  "\ud7f0": ["\u11ba", "\u11be"],
  "\ud7f1": ["\u11ba", "\u11c0"],
  "\ud7f2": ["\u11ba", "\u11c2"],
  "\ud7f3": ["\u11eb", "\u11b8"],
  "\ud7f4": ["\u11eb", "\u11e6"],
  "\ud7f5": ["\u11f0", "\u11b7"],
  "\ud7f6": ["\u11f0", "\u11c2"],
  "\ud7f7": ["\u11bd", "\u11b8"],
  "\ud7f8": ["\u11bd", "\u11b8", "\u11b8"],
  "\ud7f9": ["\u11bd", "\u11bd"],
  "\ud7fa": ["\u11c1", "\u11ba"],
  "\ud7fb": ["\u11c1", "\u11c0"],
  "\ud7dd": ["\u1105", "\u110b"]
}
with open(os.path.join(_ROOT, 'data', "decompositions.json"), 'r') as namedata:
	_JAMO_TO_COMPONENTS = json.load(namedata)
_COMPONENTS_REVERSE_LOOKUP = {tuple(comps): char for char,
							comps in _JAMO_TO_COMPONENTS.items()}

JAMO_LEADS = [chr(_) for _ in range(0x1100, 0x115F)]
JAMO_LEADS_MODERN = [chr(_) for _ in range(0x1100, 0x1113)]
JAMO_VOWELS = [chr(_) for _ in range(0x1161, 0x11A8)]
JAMO_VOWELS_MODERN = [chr(_) for _ in range(0x1161, 0x1176)]
JAMO_TAILS = [chr(_) for _ in range(0x11A8, 0x1200)]
JAMO_TAILS_MODERN = [chr(_) for _ in range(0x11A8, 0x11C3)]
JAMO_COMPOUNDS = _JAMO_TO_COMPONENTS.keys()


class InvalidJamoError(Exception):
	"""jamo is a U+11xx codepoint."""

	def __init__(self, message, jamo):
		super(InvalidJamoError, self).__init__(message)
		self.jamo = hex(ord(jamo))


def _hangul_char_to_jamo(syllable):
	"""Return a 3-tuple of lead, vowel, and tail jamo characters.
	Note: Non-Hangul characters are echoed back.
	"""
	if is_hangul_char(syllable):
		rem = ord(syllable) - _JAMO_OFFSET
		tail = rem % 28
		vowel = 1 + ((rem - tail) % 588) // 28
		lead = 1 + rem // 588
		if tail:
			return (chr(lead + _JAMO_LEAD_OFFSET),
					chr(vowel + _JAMO_VOWEL_OFFSET),
					chr(tail + _JAMO_TAIL_OFFSET))
		else:
			return (chr(lead + _JAMO_LEAD_OFFSET),
					chr(vowel + _JAMO_VOWEL_OFFSET))
	else:
		return syllable


def _jamo_to_hangul_char(lead, vowel, tail=0):
	"""Return the Hangul character for the given jamo characters.
	"""
	lead = ord(lead) - _JAMO_LEAD_OFFSET
	vowel = ord(vowel) - _JAMO_VOWEL_OFFSET
	tail = ord(tail) - _JAMO_TAIL_OFFSET if tail else 0
	return chr(tail + (vowel - 1) * 28 + (lead - 1) * 588 + _JAMO_OFFSET)


def _jamo_char_to_hcj(char):
	if is_jamo(char):
		hcj_name = re.sub(r"(?<=HANGUL )(\w+)",
						"LETTER",
						_get_unicode_name(char))
		if hcj_name in _HCJ_REVERSE_LOOKUP.keys():
			return _HCJ_REVERSE_LOOKUP[hcj_name]
	return char


def _get_unicode_name(char):
	"""Fetch the unicode name for jamo characters.
	"""
	if char not in _JAMO_TO_NAME.keys() and char not in _HCJ_TO_NAME.keys():
		raise InvalidJamoError("Not jamo or nameless jamo character", char)
	else:
		if is_hcj(char):
			return _HCJ_TO_NAME[char]
		return _JAMO_TO_NAME[char]


def is_jamo(character):
	"""Test if a single character is a jamo character.
	Valid jamo includes all modern and archaic jamo, as well as all HCJ.
	Non-assigned code points are invalid.
	"""
	code = ord(character)
	return 0x1100 <= code <= 0x11FF or\
		0xA960 <= code <= 0xA97C or\
		0xD7B0 <= code <= 0xD7C6 or 0xD7CB <= code <= 0xD7FB or\
		is_hcj(character)


def is_jamo_modern(character):
	"""Test if a single character is a modern jamo character.
	Modern jamo includes all U+11xx jamo in addition to HCJ in modern usage,
	as defined in Unicode 7.0.
	WARNING: U+1160 is NOT considered a modern jamo character, but it is listed
	under 'Medial Vowels' in the Unicode 7.0 spec.
	"""
	code = ord(character)
	return 0x1100 <= code <= 0x1112 or\
		0x1161 <= code <= 0x1175 or\
		0x11A8 <= code <= 0x11C2 or\
		is_hcj_modern(character)


def is_hcj(character):
	"""Test if a single character is a HCJ character.
	HCJ is defined as the U+313x to U+318x block, sans two non-assigned code
	points.
	"""
	return 0x3131 <= ord(character) <= 0x318E and ord(character) != 0x3164


def is_hcj_modern(character):
	"""Test if a single character is a modern HCJ character.
	Modern HCJ is defined as HCJ that corresponds to a U+11xx jamo character
	in modern usage.
	"""
	code = ord(character)
	return 0x3131 <= code <= 0x314E or\
		0x314F <= code <= 0x3163


def is_hangul_char(character):
	"""Test if a single character is in the U+AC00 to U+D7A3 code block,
	excluding unassigned codes.
	"""
	return 0xAC00 <= ord(character) <= 0xD7A3


def is_jamo_compound(character):
	"""Test if a single character is a compound, i.e., a consonant
	cluster, double consonant, or dipthong.
	"""
	if len(character) != 1:
		return False
		# Consider instead:
		# raise TypeError('is_jamo_compound() expected a single character')
	if is_jamo(character):
		return character in JAMO_COMPOUNDS
	return False


def get_jamo_class(jamo):
	"""Determine if a jamo character is a lead, vowel, or tail.
	Integers and U+11xx characters are valid arguments. HCJ consonants are not
	valid here.

	get_jamo_class should return the class ["lead" | "vowel" | "tail"] of a
	given character or integer.

	Note: jamo class directly corresponds to the Unicode 7.0 specification,
	thus includes filler characters as having a class.
	"""
	# TODO: Perhaps raise a separate error for U+3xxx jamo.
	if jamo in JAMO_LEADS or jamo == chr(0x115F):
		return "lead"
	if jamo in JAMO_VOWELS or jamo == chr(0x1160) or\
			0x314F <= ord(jamo) <= 0x3163:
		return "vowel"
	if jamo in JAMO_TAILS:
		return "tail"
	else:
		raise InvalidJamoError("Invalid or classless jamo argument.", jamo)


def jamo_to_hcj(data):
	"""Convert jamo to HCJ.
	Arguments may be iterables or single characters.

	jamo_to_hcj should convert every jamo character into HCJ in a given input,
	if possible. Anything else is unchanged.

	jamo_to_hcj is the generator version of j2hcj, the string version. Passing
	a character to jamo_to_hcj will still return a generator.
	"""
	return (_jamo_char_to_hcj(_) for _ in data)


def j2hcj(jamo):
	"""Convert jamo into HCJ.
	Arguments may be iterables or single characters.

	j2hcj should convert every jamo character into HCJ in a given input, if
	possible. Anything else is unchanged.

	j2hcj is the string version of jamo_to_hcj, the generator version.
	"""
	return ''.join(jamo_to_hcj(jamo))


def hcj_to_jamo(hcj_char, position="vowel"):
	"""Convert a HCJ character to a jamo character.
	Arguments may be single characters along with the desired jamo class
	(lead, vowel, tail). Non-mappable input will raise an InvalidJamoError.
	"""
	if position == "lead":
		jamo_class = "CHOSEONG"
	elif position == "vowel":
		jamo_class = "JUNGSEONG"
	elif position == "tail":
		jamo_class = "JONGSEONG"
	else:
		raise InvalidJamoError("No mapping from input to jamo.", hcj_char)
	jamo_name = re.sub(r"(?<=HANGUL )(\w+)",
					jamo_class,
					_get_unicode_name(hcj_char))
	# TODO: add tests that test non entries.
	if jamo_name in _JAMO_REVERSE_LOOKUP.keys():
		return _JAMO_REVERSE_LOOKUP[jamo_name]
	return hcj_char


def hcj2j(hcj_char, position="vowel"):
	"""Convert a HCJ character to a jamo character.
	Identical to hcj_to_jamo.
	"""
	return hcj_to_jamo(hcj_char, position)


def hangul_to_jamo(hangul_string):
	"""Convert a string of Hangul to jamo.
	Arguments may be iterables of characters.

	hangul_to_jamo should split every Hangul character into U+11xx jamo
	characters for any given string. Non-hangul characters are not changed.

	hangul_to_jamo is the generator version of h2j, the string version.
	"""
	return (_ for _ in
			chain.from_iterable(_hangul_char_to_jamo(_) for _ in
								hangul_string))


def h2j(hangul_string):
	"""Convert a string of Hangul to jamo.
	Arguments may be iterables of characters.

	h2j should split every Hangul character into U+11xx jamo for any given
	string. Non-hangul characters are not touched.

	h2j is the string version of hangul_to_jamo, the generator version.
	"""
	return ''.join(hangul_to_jamo(hangul_string))


def jamo_to_hangul(lead, vowel, tail=''):
	"""Return the Hangul character for the given jamo input.
	Integers corresponding to U+11xx jamo codepoints, U+11xx jamo characters,
	or HCJ are valid inputs.

	Outputs a one-character Hangul string.

	This function is identical to j2h.
	"""
	# Internally, we convert everything to a jamo char,
	# then pass it to _jamo_to_hangul_char
	lead = hcj_to_jamo(lead, "lead")
	vowel = hcj_to_jamo(vowel, "vowel")
	if not tail or ord(tail) == 0:
		tail = None
	elif is_hcj(tail):
		tail = hcj_to_jamo(tail, "tail")
	if (is_jamo(lead) and get_jamo_class(lead) == "lead") and\
	(is_jamo(vowel) and get_jamo_class(vowel) == "vowel") and\
	((not tail) or (is_jamo(tail) and get_jamo_class(tail) == "tail")):
		result = _jamo_to_hangul_char(lead, vowel, tail)
		if is_hangul_char(result):
			return result
	raise InvalidJamoError("Could not synthesize characters to Hangul.",
						'\x00')


def j2h(lead, vowel, tail=0):
	"""Arguments may be integers corresponding to the U+11xx codepoints, the
	actual U+11xx jamo characters, or HCJ.

	Outputs a one-character Hangul string.

	This function is defined solely for naming conisistency with
	jamo_to_hangul.
	"""
	return jamo_to_hangul(lead, vowel, tail)


def decompose_jamo(compound):
	"""Return a tuple of jamo character constituents of a compound.
	Note: Non-compound characters are echoed back.

	WARNING: Archaic jamo compounds will raise NotImplementedError.
	"""
	if len(compound) != 1:
		raise TypeError("decompose_jamo() expects a single character,",
						"but received", type(compound), "length",
						len(compound))
	if compound not in JAMO_COMPOUNDS:
		# Strict version:
		# raise TypeError("decompose_jamo() expects a compound jamo,",
		#                 "but received", compound)
		return compound
	return _JAMO_TO_COMPONENTS.get(compound, compound)


def compose_jamo(*parts):
	"""Return the compound jamo for the given jamo input.
	Integers corresponding to U+11xx jamo codepoints, U+11xx jamo
	characters, or HCJ are valid inputs.

	Outputs a one-character jamo string.
	"""
	# Internally, we convert everything to a jamo char,
	# then pass it to _jamo_to_hangul_char
	# NOTE: Relies on hcj_to_jamo not strictly requiring "position" arg.
	for p in parts:
		if not (type(p) == str and len(p) == 1 and 2 <= len(parts) <= 3):
			raise TypeError("compose_jamo() expected 2-3 single characters "
							"but received " + str(parts),
							'\x00')
	hcparts = [j2hcj(_) for _ in parts]
	hcparts = tuple(hcparts)
	if hcparts in _COMPONENTS_REVERSE_LOOKUP:
		return _COMPONENTS_REVERSE_LOOKUP[hcparts]
	raise InvalidJamoError(
		"Could not synthesize characters to compound: " + ", ".join(
			str(_) + "(U+" + str(hex(ord(_)))[2:] + ")" for _ in parts), '\x00')


def synth_hangul(string):
	"""Convert jamo characters in a string into hcj as much as possible."""
	raise NotImplementedError
	return ''.join([''.join(''.join(jamo_to_hcj(_)) for _ in string)])