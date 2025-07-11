# Hangul-to-QWERTY Conversion Test Cases
# 한글-QWERTY 변환 테스트 케이스

## Basic Test Cases (기본 테스트 케이스)

| Hangul | Expected QWERTY | Description |
|--------|----------------|-------------|
| 가     | rk             | ㄱ + ㅏ |
| 윤     | dbs            | ㅇ + ㅠ + ㄴ |
| 하늘   | gksrmf         | 하(ㅎ+ㅏ) + 늘(ㄴ+ㅡ+ㄹ) |
| 되     | enl            | ㄷ + ㅚ (ㅗ+ㅣ) |
| 돼     | eho            | ㄷ + ㅙ (ㅗ+ㅐ) |
| 맑     | akfr           | ㅁ + ㅏ + ㄺ (ㄹ+ㄱ) |

## Edge Cases (예외 케이스)

| Hangul | Expected QWERTY | Description |
|--------|----------------|-------------|
| 띄     | Eml            | ㄸ(double ㄷ) + ㅢ(ㅡ+ㅣ) |
| 넓     | spfq           | ㄴ + ㅓ + ㄼ(ㄹ+ㅂ) |
| 괜     | rhso           | ㄱ + ㅙ(ㅗ+ㅐ) + ㄴ |
| 뜨     | Emt            | ㄸ + ㅡ |
| 씨     | Tl             | ㅆ(double ㅅ) + ㅣ |

## Complex Vowel Tests (복합 모음 테스트)

| Vowel | Decomposition | QWERTY |
|-------|---------------|--------|
| ㅘ    | ㅗ + ㅏ       | hk     |
| ㅙ    | ㅗ + ㅐ       | ho     |
| ㅚ    | ㅗ + ㅣ       | hl     |
| ㅝ    | ㅜ + ㅓ       | nj     |
| ㅞ    | ㅜ + ㅔ       | np     |
| ㅟ    | ㅜ + ㅣ       | nl     |
| ㅢ    | ㅡ + ㅣ       | ml     |

## Complex Consonant Tests (복합 자음 테스트)

| Consonant | Decomposition | QWERTY |
|-----------|---------------|--------|
| ㄳ        | ㄱ + ㅅ       | rt     |
| ㄵ        | ㄴ + ㅈ       | sw     |
| ㄶ        | ㄴ + ㅎ       | sg     |
| ㄺ        | ㄹ + ㄱ       | fr     |
| ㄻ        | ㄹ + ㅁ       | fa     |
| ㄼ        | ㄹ + ㅂ       | fq     |
| ㄽ        | ㄹ + ㅅ       | ft     |
| ㄾ        | ㄹ + ㅌ       | fx     |
| ㄿ        | ㄹ + ㅍ       | fv     |
| ㅀ        | ㄹ + ㅎ       | fg     |
| ㅄ        | ㅂ + ㅅ       | qt     |

## Challenging Distinctions (어려운 구분)

### 되 vs 돼
- 되: ㄷ + ㅚ (ㅗ+ㅣ) → "enl"
- 돼: ㄷ + ㅙ (ㅗ+ㅐ) → "eho"

### Double Consonants (된소리)
- ㄲ: "R" (shift + r)
- ㄸ: "E" (shift + e)
- ㅃ: "Q" (shift + q)
- ㅆ: "T" (shift + t)
- ㅉ: "W" (shift + w)

## Full Word Tests (전체 단어 테스트)

| Word | Expected QWERTY | Breakdown |
|------|----------------|-----------|
| 안녕하세요 | dkssudgksshdy | 안(d+k+s) + 녕(s+u+d) + 하(g+k) + 세(t+p) + 요(d+y) |
| 사랑해   | tkfkdgo       | 사(t+k) + 랑(f+k+d) + 해(g+o) |
| 고마워   | rhakdj        | 고(r+h) + 마(a+k) + 워(d+j) |
| 괜찮아   | rhstcdk       | 괜(r+h+s) + 찮(c+k+x) + 아(d+k) |

## Test Commands for ESP32

```
testhangul     # Run all basic tests
testko1        # Test "가윤" → "rkdbs"
testko2        # Test "되돼맑" → "enlehoakfr"
testko3        # Test "띄넓" → "Emlspfq"
```

## Validation Notes

1. **UTF-8 Handling**: Properly decode 3-byte Korean characters
2. **Syllable Range**: Only process 0xAC00-0xD7A3 range
3. **Mixed Text**: Handle Korean + English + punctuation
4. **Error Recovery**: Skip invalid characters gracefully
5. **Memory Efficiency**: Use stack allocation for temporary strings