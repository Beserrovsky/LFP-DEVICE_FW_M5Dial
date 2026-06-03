# UI/UX Flow

#### Flow 1: Home/Identification
Screen: Home

Waits for NFC read OR (for debugging) 5 clicks

If NFC reading fails (ID not in correct format) - Go to WaitOrClick page with

lblWaitOrClick ="Error!\n\n{message}". message must be at most 37 chars.

#### Flow 2: Tutorial + Questions
Screen: Questions

1. First, a two page tutorial,

lblQuestionsType = "Tutorial"

arcQuestionsProgress.value = 0
lblQuestionsNumeration = "1/2"
lblQuestionsInstruction begins with "rotate the dial until OK",

lblQuestionsCurrentOption and lblQuestionsPrevOption starts as "X"
lblQuestionsNextOption starts as "OK"

Next to it or prior are blank, not accessible.

Once user rotates, lblQuestionsInstruction = "click once to advance". If they rotate back, go back to "rotate the dial until OK."

If they advance,
lblQuestionsNumeration = "2/2", 
arcQuestionsProgress.value = 1, 
5 rotation options "X", next to it or prior are blank, not accessible. 

lblQuestionsInstruction = "click twice to go back", once that happens, 

arcQuestionsProgress.value = 0
lblQuestionsNumeration = "1/2"
only one option = "OK"
lblQuestionsInstruction = "click once again to end tutorial".

2. Then, 4 questions...

lblQuestionsType = "Question"

Same UI, but now the Questions, lblQuestionsInstruction = Q1 text.... 

Q_number = 1, 2, 3 or 4 = Number of the current question, confirmation is 5/5

arcQuestionsProgress.value = Q_number or T_number-1
lblQuestionsType = "Question" or "Tutorial"
lblQuesstionsNumeration = "{Q_number}/5" or "{T_number}/2"
lblQuestionsInstruction = 37 characters -> Actual question or Tutorial Instruction - Example: "How would you rate J&J on innovation?"

Screen: Confirm

For the 5/5 question, another screen is evoked - "Confirm" - There:
- Two clicks go back to 4/5 question
- One click sends the data and goes to a WaitOrClick page

#### Flow 3: End or retry
Screen: WaitOrClick
The last WairOrClick depends on the response of the package. 

1. If everything goes well:

lblWaitOrClick ="Thanks!\n\nFind yourself at the bigger screen."

Once the timers runs out or user clicks, go to Home

2. If an error occurs:

lblWaitOrClick ="Error!\n\n{message}". message must be at most 37 chars.

Once the timer runs out or user clicks, go to Confirm page again.

#### Other things to keep in mind:

- Hold the button for at least 2 seconds - beeps twice - Mirror display imediattly (no release required)
- Hold the button for 5 seconds - beeps three times - Device restart imediattly (no release required)