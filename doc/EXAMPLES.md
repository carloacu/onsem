# onsem examples


## Voice only features


### Information retrieval based on heard information


The bot is constantly store information while chatting to be coherent and to be able to restitue the information.

```
user "ask who likes Paul"
bot: "Who likes Paul ?"
user: "Toto"
sentiment_from_user_input: [human, sentiment_positive_any, 2, human, asr]
user: "Toto aime Paul ?"
bot: "Oui, Toto aime Paul."
user: "Paul aime Toto ?"
bot: "Je ne sais pas si Paul aime Toto."
```



### Sentiments extraction


The sentiments are tracked in the text heard.<br/>
The sentiment is an array of 5 values:<br/>
1) The identifiant of who is the author of the sentiment.
2) What is the type of the sentiment.
3) What is the strengh of the sentiment.
4) The identifiant of the receiver of the sentiment.
5) How the sentiment was transmitted.

Here is an example:

```
user: "I like you"
sentiment_from_user_input: [currentUser, sentiment_positive_any, 2, robot, asr]
bot: "Thanks"
user: "je t'aime"
sentiment_from_user_input: [currentUser, sentiment_positive_any, 3, robot, asr]
bot: "Merci"
```


The sentiments can also be extracted from the context:

```
user: "demande-moi si je suis sympa"
bot: "Est-ce que tu es sympa ?"
user: "oui"
sentiment_from_user_input: [currentUser, sentiment_positive_any, 4, currentUser, asr]
```


Then Sentiments extraction can conbine inputs from different languages

```
user: "ask me in French if I am happy"
bot: "Est-ce que tu es heureux ?"
user: "oui"
sentiment_from_user_input:: [currentUser, sentiment_positive_any, 5, currentUser, asr]
```



## Multimodal features


### Managing information brought by events

The external events can be linked to an information.<br/>
In the following example, when "EngagedWithUserId" is raised it means that the bot is engaged with a user that have the identifiant equals to the value of the event.

```
event: <"EngagedWithUserId", "2">
user: "I am Erwan"
event: <"EngagedWithUserId", "1">
user: "demande à Erwan en anglais ce qu'il fera ce soir"
bot: "Ok, je demanderai Erwan qu'est-ce que Erwan fera ce soir."
event: <"EngagedWithUserId", "2">
bot: "What you will do this evening ?"
user: "I will go to cinema"
event: <"EngagedWithUserId", "1">
user: "qu'est ce que Erwan va faire ce soir ?"
bot: "Erwan ira au cinéma ce soir."
```


## Action management

Some voice requests can be linked to actions.

```
user: " lève la main gauche"
bot: "^moveArmUp("Left")"
user: "raise your right arm"
bot: "^moveArmUp("Right")"
user: "si tu me vois, lève la main droite"
bot: "Ok, je lèverai la main droite si je te vois."
user: "tu me vois"
bot: "^moveArmUp("Right")"
user: "lève les bras"
bot: "^moveArmUp("Left") ^moveArmUp("Right")"
```



## Dynamically connects actions to events


You can dynamically link by voice actions to events.

```
event: <"EngagedWithUserId", "1">
user: "lève-toi à chaque fois que je souris"
bot: "Ok, je me lèverai à chaque fois que tu souris."
event: <"PersonSmiling", "true">
bot: "^goToPosture("Stand")"
user: ">>15s" // Or wait 15 seconds, it an easter egg in the gui to simulate time elapsing.
event: <"PersonSmiling", "true">
bot: "^goToPosture("Stand"))"
user: ">>15s" // Or wait 15 seconds, it an easter egg in the gui to simulate time elapsing.
event: <"PersonSmiling", "true">
bot: "^goToPosture("Stand"))"
```

Or

```
user: "lève-toi si quelqu'un touche ta main"
bot: "Ok, je me lèverai si quelqu'un touche ma main."
event: <"AnHandIsTouched", "true">
bot: "^goToPosture("Stand")"
user: "stand up if someone touch your left hand"
bot: "Ok, I will stand up if someone touches my left hand."
event: <"AnHandIsTouched", "false">
event: <"AnHandIsTouched", "true">
bot: "^goToPosture("Stand")"
```

