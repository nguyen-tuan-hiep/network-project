## Protocol


## Elo Calculating Formular

### Explain

1. Estimating elo diff: Loser - Winner
e.g. 1810-1750 = 60

2. Divide it by 400
e.g.  = 60/400 = 0.15

3. Raise ten to power of it and add one:
e.g. 1 + 10^0.15 = 2.413

4. Find the expected score by working out the latter's multiplicative inverse
expected score=1/2.413=0.414

5. Evaluate the rating change according to the Elo rating formula, whereas 32 is K-factor, for different Elo, K-factor will be different
32 * (1−0.414)=11.7

### Final formula
elo for A : K * (result - 1/(1+10 ^ (B_elo - A_elo / 400)))
elo for B : K * (result - 1/(1+10 ^ (A_elo - B_elo / 400)))

### K-factor
K = 32      below 2100
K = 24      between 2100 and 2400
K = 16      above 2400

### References 
https://www.omnicalculator.com/sports/elo#elo-calculator-in-practice-elo-rating-in-a-chess-tournament

https://en.wikipedia.org/wiki/Elo_rating_system