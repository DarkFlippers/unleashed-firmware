# flipperzero-yatzee
Yahtzee game for flipperzero

Its not beautiful, but it works and now I can play Yahtzee on my flipper while I shit at work.

<b>Installation: </b>

Download fap from releases. Copy fap to flipper sd at <b>ext > apps > Games</b>


<b>Controls: </b>

- Up to Roll
- Left/Right to move cursor
- OK to Hold a die
- Moving cursor past the dice will move the cursor up to the scorecard. Moving the scores cursor will show you the potential score you would get.


<b>Rules & Scoring:</b>

- Between rolls, move the cursor and use the OK button to select which dice you will hold for the next roll
- 3 rolls per round and then you are forced to select a score. 
- To score, move cursor with Left/Right up to the scorecard, when desired score to count is underlined, press the Down button to confirm.

- 1-6 add up the corresponding dice of that number in your roll.
- 3 of a Kind (3k) = total of dice when 3 of a kind is rolled
- 4 of a Kind (4k) = total of dice when 4 of a kind is rolled
- Full House (Fh) = 25
- Small Straight (Sm) = 30
- Large Straight (Lg) = 40
- Chance (Ch) = total of all dice in roll
- Yatzee (Yz) = 50 for the first yatzee. Successive Yatzees do not show in the score card, but add 100 each to the total score
- Game ends when every scoring value has been selected once.
- If sub score is at least 63, 35 points are added to the total score.

<b>Todo</b>
- Redo the scorecard now that I understand a little better how this works
  - Would like to make it a grid with 4 rows, and 4 columns
    - Rows 0 and 2 will have the scores 'names'
    - Rows 1 and 3 will be empty until filled by a score.
    - Column 3 will span all rows and show each bonus yatzee as an icon like a star or something.
  - Once grided score card is implemented, identify a better mechanism to show that a score has already been counted- instead of the '.' that shows up now.
    - Maybe invert the grid color to show which score is being selected by the cursor.
- If upper score >= 63, add a pop-up message at the end game to give a visual indication that a bonus for the top row is being.
- Redo button mapping so that 
  - the middle button can be used to both hold on dice or confirm score. Leaves the down arrow open for something else.
  - Would be nice if up/down could be used to move through the scorecard and left/right move through the dice, but then theres not enough buttons for ROLL so idk yet.
- Learn more about C so that I can move stuff to a header file like everyone else does.
