# How onsem works


In onsem there is basically 3 parts:<br/>
 1) Convert a text to a semantic tree
 2) Reasonate on the semantic trees, we will explain here the structure of a semantic tree
 3) Convert a semantic tree to a text


## Convert a text to a semantic tree


This algorithm will first try to extract the chunks of the input text and then link the chunks grammatically and then fill the resulting semantic tree.<br/>
We can split, further in 5 steps that we will detail.


### Step1: Tokenization and part of speech detection

Thanks to our linguistic resources manually done, the engine knows the different part of speeches and inflections of each word of the input text.<br/>
The engine will try to reduce the number of part of speech possibilities by applying successively different kind of algorithms.<br/>
Some algorithms remove the impossible successions of part of speeches. (ex: a preposition followed by an interjection)<br/>
Some algorithms check and remove if necessary, some successions based on the inflections. (ex: a determiner followed by a noun is possible only if the inflections of the determiner and the noun have certain characteristics)<br/>
Some algorithms track some part of speech patterns over many words to remove the impossible part of speeches. (ex: each determiner part of speech not followed by a noun or a proper noun, with eventually some adjectives in between, are removed)


### Step 2: Create grammatically linked chunks

This step starts by putting all the words (already tokenized with an identified part of speech, cf step 1) of the input text inside a single chunk.<br/>
After that, the engine will apply different kind of algorithms in order to divide this big chunk into smaller ones.<br/>
And the engine will link to one another the chunks according to the grammatical structure of the input text.<br/>
The first algorithm will split the verbal groups, the conjunctions and the punctuations into separate chunks.<br/>
The second algorithm will split the remaining chunks in order to links the subjects, objects and subordinates chunks to the verbal chunks.<br/>
The third algorithm will focus on identifing more precisely the kind of information bring by each verb children. (ex: time, location, mitigation, ...)<br/>
The fourth algorithm will rearrange the children links according to a more high level structure of the text.



### Step 3: Error detection and, if necessary, go back to the first step with a reduced set of possible part of speeches

This step tries to detect some errors in the current grammatical structure.<br/>
If a pattern is not possible (ex: a verb cannot be at the imperative from but has no subject), we try to remove a part of speech causing that and we go back to the step 1.


### Step 4: Replace some pronouns by their referred nodes

This step will try to replace some pronouns that can be replaced from the information contained in the current text. (ex: "I like Paul. He is tall" the "he" node is replaced by the "Paul" node).


### Step 5: Convert the grammatically linked chunks to a semantic tree (cf. "Semantic tree" chapter)

This step is composed of a single algorithm that converts the linked chunks to a semantic tree.<br/>
A semantic tree reflects more the meaning of a text (condition, equality between elements, ...) than the linked chunks.<br/>
For more details, please look at the "Semantic tree" chapter.




## Semantic tree


The semantic tree is a representation of the "meaning" of a sentence.<br/>
It doesn't try to represent how a sentence is structured but how its contained information can be programmatically used.<br/>
The goal of this representation is:<br/>
 * To allow logical reasoning
 * To provide enough information to allow a natural language synthesis

The semantic tree is composed of structural and grounding nodes.<br/>
We will take a look at each of these kind of nodes.


### Structural node


The structural nodes specify how each piece of information contained in the input text are linked together.<br/>
It looks like an AST (Abstract Syntax Tree) of a programming language with conditions, loops, set of elements, comparisons between elements, ...

A structural node can be:
 * A comparison
 * A condition
 * A list
 * An interpretation
 * A feedback
 * A set of forms
 * A node containing a grounding

We will see them more in details right below.


#### A comparison

A comparison structure can be composed of 3 expressions:
 * 2 of them define the left and right operands
 * the third one defines what is being compared

The right operand is not mandatory, in that case a superlative is represented.<br/>
The expression that defines what is compared is not mandatory either, another attribute that define the type of the comparison (lesser, greater, equal, ...) can be enough.


#### A condition

A condition structure can be composed of 3 expressions:
 * A condition to check
 * An information that becomes valid when the condition is true
 * An information that becomes valid when the condition is false. This expression is not mandatory.


#### A list

A list structure is composed of expressions separated by a logical connector ("and" or "or" basically)


#### An interpretation

An interpretation structure exposes an initial expression and a new expression representing how the initial expression should be considered according to the context.<br/>
The context is composed of the other expressions from the same container (It is what we call a Memory).


#### A feedback

A feedback structure contains 2 exressions:
 * An expression of the central information
 * An exression corresponding to a feedback of the central information. (Ex: "Nice", "Cool", ...).


#### A metadata

A metadata strucutre provides some contextual information like:
 * Who is the author
 * Who is the recever
 * What is the extractor that provides the information (ASR, TTS, specific sensor detection, written text, ...)
 * What is the original text
 * What is the original text language


#### A set of forms

A set of forms is a structure that is used for the questions. It provides other associated questions that need to be answered with an order of priority.<br/>
The engine tries to answer to the questions that have the highest priority.<br/>
If the answer is found, the chatbot will stop here.<br/>
If the answer is not found the engine will try to answer to the question that have the second highest priority, and so on until the lowest priority.<br/>

#### A node containing a grounding

A grounded structure contains a grounding node (cf below) and potentially some children expression labeled as "subject", "object", "time", "location", ....




### Grounding node


Each each leaf of a semantic tree is an grounded node without any child.<br/>
Every piece of information contained in the input text are stored in a grounding node.<br/>
There is many kind of groundings:
 * Agent
 * Duration
 * Generic
 * Language
 * Parameter
 * RelativeLocation
 * Statement
 * Text
 * Time
 * Url
 * Concept


#### Agent

The Agent grounding contains a unique identifier that points to a user or a robot.


#### Duration

The Duration grouding defines an offset of time.


#### Generic

The Generic grouding is the commonly used Grounding. It is used every time an information cannot be stored in any more specific grouding.

A Generic grounding stores:
 * A word in a specific language
 * Some related concepts
 * The number of instances (ex: "a banana" = 1, "two bananas" = 2, "some bananas" = plural with an undefined value)
 * If we are talking about a specific, any or an undefined instance (ex: "a banana" = undefined instance, "the banana" = specific instance, "bananas" = any instance)
 * If this grounding needs to be replaced by something recent in the context (ex: the pronouns when sentence context allows it)


#### Language

The Language npde, contians a language identifier (ex: ENGLISH, FRENCH, ...).


#### Parameter

The Parameter grounding contains a parameter identifier and a grounding type.<br/>
It is a grounding that can be replaced by a semantic tree (value of the parameter).


#### RelativeLocation

A Relative Location grounding defines an offset of location. (ex: NEXT_TO, FAR_FROM, ...)


#### Statement

A statement grounding contains a verb with some concepts a tense.


#### Text

A Text grounding contains a raw text, with eventually a language associated.


#### Time

A Time grounding refers to a specific time, defined by a duration from January 1, 0.


#### Url

An Url grounding contains an url and eventually the type of content pointed by the url. (ex: video, site, image, ...)


#### Concept

A Concept grounding, is the simplest grounding, it can only contains some concepts. (like any grouding)






## Convert a semantic tree to a text


The synthesizer works in 3 steps:
1) The synthesizer detects the verbal and isolated nominal nodes
2) The synthesizer tries to synthesize their linked sub-trees
3) The synthesizer merges them in the good order



### Step 1: Visit the semantic tree in order to detect the verbal phrases or the nominal groups to synthesize

The goal is to cut the semantic tree into smaller trees that will be given in input of the step 2.<br/>
The considered smaller trees will be:
 * Each verbal node with it's children. (except the sub verbal nodes)
 * Each nominal node, that is not itself a child of a verbal or nominal node, with it's children.



### Step2: Synthesize each part of a verbal phrase

The goal is to synthesize separately each node of this input tree.



### Step 3: Put the synthesized chunks in the good order

The synthesizer will concatenate the synthesized nodes in the most appropriate order possible according the meta datas and the tree structure.

Example:
The algorithm merge those informations:
Affirmative sentence
Language: English
Verb synthesization: “liked”
Subject synthesization: “you”
Object synthesization: “chocolate”
To generate this output sentence:
“You liked chocolate”

