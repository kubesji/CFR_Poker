# CFR_Poker

My implementation of Monte Carlo Counterfactual Regret with external sampling algorithm to solve Texas Hold'em Poker.

The goal is to have program that is capable to train full game with 6 player but currently supports only 2 players. Also, due to lacking computational power, I was only able to train a model on game without river card.

The algorithm was originally written in Python but was redone in C++ because of speed. Or rather lack of it. Significant speed improvement was achieved thanks to poker hand lookup tables. 

## MCCFR with external sampling

Algorithm used is described [here](https://www.mlanctot.info/files/papers/PhD_Thesis_MarcLanctot.pdf), pseudo code in *Algorithm 4*. For details, please read the thesis. Main difference compared to vanilla MCCFR is that only *hero's* actions are explored while *chance* and *opponent's* actions are sampled. Game tree is not explored fully each game but, maybe contrary to intuition, it is empirically shown that this method converges faster to Nash's equilibrium.

## State space

Game state is composed of two parts: player's cards and history. Player's cards is set of cards in player's hand as well as on the table. History is list of actions taken by each player. Both parts can be massive in size, thus, some sort of compression or state space size reduction is needed. While reducing number of states in the most important thing here, it is also good to keep in mind there can easily be 10^9 states, thus, the shorter state string, the better for memory.

## Compressing player's hands

There is enourmous amount of poker hands and exploring each and every one of them is not in power of any computer. For one poker hand, we need to traverse whole game tree which generates whole heap of states - one hand can generate hundreds different game states depending on number of players. This is why compressing game states is important. Training time can be significantly reduced by compressing more states into one. This has to be done carefully as loss of information caused by compression can be detrimental to training effort.

One such example is treating pair of twos and pair of threes as identical state. Agent would not significantly change strategy having one or the other combination so it makes sense to compress them into one state even if we loose information.

Preflop cards are compressed in lossless way. Each player is dealt two cards. State is three character string composed as sorted values of both cards followed by characted *u* for unsuited cards and *s* if cards are of the same suit. For instance, 5♠ and 7♣ will be turned into *57u*.

### Explanation in example

Player has 5♠ and 7♣ in hand while there are 4♠, 6♦, 8♦, 10♦ and 10♥ on board. Thus, player has straight. If the hand was stored as string of cards, there are millions of combinations just for straights. Game state string would look something like this: *"5♠7♣-4♠6♦8♦T♦T♥"* (T is for 10). So, we need to reduce it but loose as little information as possible. 

The first step is to remove suits from card as they are not important for straights. Or are they? In this case, there are three diamond on the table, meaning other player can have flush (or even straight flush in this case). But rather than keeping suit for each card, we can add a flag *F* such as

$$
F = max \left( \sum_{c \in T} ♠(c), \sum_{c \in T} ♦(c), \sum_{c \in T} ♥(c), \sum_{c \in T} ♣(c) \right)

$$

where *T* is set of all cards on table and function *♥(x)* is defined as

$$
♥(x) = \left\{ 
  \begin{array}{ c l }
    1 & \quad \textrm{if } F \in ♡ \\
    0 & \quad \textrm{otherwise}
  \end{array}
\right. 
$$
where ♡ is set of all cards having heart suit. Similar applies to other suit functions.

Or, since we know danger of suit starts at 3 suited cards of the table, we can use boolean flag
$$
F_b =\left\{ 
  \begin{array}{ c l }
    True & \quad \textrm{if } F \geq 3 \\
    False & \quad \textrm{otherwise}
  \end{array}
\right.

$$

This leaves us with game state string as *"57-468TT-t"* where *t* is for True. This simple trick reduced number of states to explore by whopping 3456. Therefore, it is 3456 times faster to learn how to play if agent has a straight. But there is more.

Next, there is 10 different ways to deal straight of the same value. We can try to reduce this. Simple way would be just to write *Straight8-TT-t* or even simpler *S8-TT-t* for memory efficiency. But this approach comes with information loss. Although agent doesn't need to know if it has 4 or 5 in hand but he needs to know if the highest card is on table or in its hand. Having the highest card in hand gives player higher chance of winning. Therefore, we can add another flag
$$
H_S = \left\{ 
  \begin{array}{ c l }
    True & \quad \textrm{if } \exists c \in H \text{ such as } c=max(S) \\
    False & \quad \textrm{otherwise}
  \end{array}
\right.
$$
where *H* is set of cards in player's hand and *S* is set of card from straight combination. This flag could be expanded to numerical value 0, 1 or two denoting how many highest card player hold in hand. We'll stick with boolean for the sake of the example. Reduction in this case is only 5. But hey, still a great number. This leaves us with state string *S8-TT-tf* 

The last thing we want to get rid of are two unused card which other players can use for their benefit, in this case two tens. There is 2070 possible combinations but we cannot just discard them. A pair on table means any player lucky enough can have a full house or even four of a kind. Thus, lacking this information would leave agent vunerable to this particular scenario no matter how unlikely it is. Since full house or four of a kind beating straight is very rare, agent would learn it is very likely to win with straight beating pairs or whatnot and it might place a high bet having no information about possible higher combination. Lets introduce a value that is a count of cards of the same value on the table that can be written as
$$
N_v = \sum_{c \in T} 1 - \left|sign(c-v)\right|
$$
where $v \in V$ and V is set of all card values. From there, flags *P* and *ToK* can be defined as
$$
P = \left\{ 
  \begin{array}{ c l }
    True & \quad \textrm{if } \exists v \in V \text{ such as } N_v=2 \\
    False & \quad \textrm{otherwise}
  \end{array}
\right.
,
ToK = \left\{ 
  \begin{array}{ c l }
    True & \quad \textrm{if } \exists v \in V \text{ such as } N_v=3 \\
    False & \quad \textrm{otherwise}
  \end{array}
\right.
$$
Alternatively, flag *P* can have numerical values ${0, 1, 2}$ coresponding to number of pairs on table rather just a boolean flag. This reduces number of states by 517.5 or 346 to 4 or 6 respectivelly. All in all, we reduced state space nearly by nine million states and that's before we include any player moves.

Similar approach can be taken to flag up to agent on flop and turn rounds that there might be better card combination coming for the agent by repeating shown algorithms and including cards from hand. This will result in higher state space but lower information loss which is always a trade-off we have to consider.

Performing so many operations and checks on each iterations would lead to extremely long trainign times. To speed things up, look-up table are utilised here as mentioned in the beginning.

### Look-up tables

Doing all these thing programatically would be extremely computationally expensive. Moreover, most of it is repetitive and there is no point to do the exact same operations every iteration. This is what lookup table are great for. Every possible card combination is evaluated beforehand, assigned unique hash and stored in big table. This approach was heavily inspired by [PokerHandEvaluator](https://github.com/HenryRLee/PokerHandEvaluatorhttps:/). Base algorithm to evaluate hand is the same but additional information is returned. This information helps with compressing state space but keeps as much information as possible. The original algorithm is focused on determinig a winner and ignores remaining two cards and doesn't provides little to no information about possible better combinations.

## Action history

Once player's cards has been compressed, action history is to be added to the state string. There are four actions in poker: fold, check, call & raise. Since this algorithm is tabular and raise is not discrete (can raise by arbitrary value), it needs to be discretised. The finer discretisation, the more actions and longer training time. Player does never have check and call actions available at the same time - number of actions can be reduced by merging either call&check into call action or fold&check into passive action.

Number of states grow exponentionally with number of players. Depending on discretisation of raise action, there can be tens to hundres of states within one game round in full game tree of two player game. It grows to thousands for 6 player game. Game consists of 4 betting rounds, therefore number of states generated within one game skyrockets by power of 4. It raises need to compress it as well.

We can keep just the last action for each player but it provides very little information. Next level would be to keep all actions from given round and discard action from previous rounds. This is better but we loose information who was aggressive and who was passive. The lowest level of compression is to remember last action of a round for each player. This approach is used here. Every player is assigned one letter for last action:
- *p* for fold/check;
- *c* for call of someones raise;
- *C* call reraise of players raise;
- *r* for raise that was called or folded by others;
- *R* for reraise that was called or folded by other players.