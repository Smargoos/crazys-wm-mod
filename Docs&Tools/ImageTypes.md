An image pack provides a set of image (or animation) files as well as a
mapping from the **files** to **image resources**. In the most simple case,
this mapping is achieved by the file names. Newer packs should define the
mapping using an xml file.

Whenever the game looks for an image to display, it goes over all **image resources**
over that pack. Each **image resource** defines the following:
* a list of image files that are used for this resource
* a primary category
* potentially secondary tags or the absence thereof
* additional conditions

In this setting, the primary category corresponds to the image category of the old
code (i.e. maid/strip/sex...). The secondary tags are a generalization of the image
variations that existed before for pregnant girls. Finally, the conditions are 
generic conditions that can specify requirements when the images are allowed or
forbidden.

As an example, consider the following image resource:
- files: "maid \[slave\] 1.png", "maid \[slave\] 2.png"
- primary category: "maid"
- secondary tags: \[slave\]
- conditions: EMPTY

This makes this the preferred image resource when looking up maid images for
slave girls. 

Given the vast number of combinations of primary and secondary tags, we cannot 
expect packs to fill in all these options. Therefore, a good fallback strategy 
is needed.

We also want to make sure that the images are displayed with enough variety. Therefore,
we rate all image resources according to how well they fit to the current process,
and then take the top rated ones such that we have at least 10 or so images available,
from which we then randomly choose.

The rating system works as follows:
* an image which is disabled due to conditions has minus infinity as a score.
* if a resource with the exact primary and secondary category exists, this has a score of
zero.
* otherwise, we look at manipulated versions of the resource. These are 
  - the resource but with a different primary tag but the same secondary tags
  - the resource with the same primary but different secondary tags
  - a different primary and a different secondary tag. 
  - Same tags, but looked up for another girl or the default images 
This change in resource is associated with a cost that gets subtracted from 
the score of the new resource.
  
The fallback mechanism can thus be specified by providing substitution rules with associated
costs. To keep things easily mod-able, the substitution rules are defined in an xml file.
A substitution rule consists of two things:
* A specification for the resource to which it applies
* A specification how to change the resource
* A cost value

The resource specification consists of a primary tag or a list of primary tags,
and a specification for secondary tags to be either present or absent.
The change rule can define a new primary tag, and change which secondary tags have
to be present or absent or are ignored.

For example, the following rule
```xml
<Substitution Cost="10">
    <Match>
        <Primary>cleaning</Primary>
    </Match>
    <Change>
        <Primary>maid</Primary>
    </Change>
</Substitution>
```
would imply that the following changes are allowed with a cost of 10:
* cleaning -> maid
* cleaning \[slave\] -> maid \[slave\]
* cleaning \[pregnant\] -> maid \[pregnant\]
* etc.

whereas a rule such as
```xml
<Substitution Cost="5">
    <Match>
        <Primary>cleaning</Primary>
    </Match>
    <Change>
        <Primary>cleaning</Primary>
        <IgnoreSecondary>*</IgnoreSecondary>
    </Change>
</Substitution>
```
would enable the following cost 5 rules
* cleaning -> cleaning \[slave\]
* cleaning \[slave\] -> cleaning
* cleaning \[pregnant\] -> cleaning
* etc.

by going multiple steps, these two rules also allow for the substitutions
* cleaning \[slave\] -> maid (15)
* cleaning -> cleaning \[slave, pregnant\] (10)
* cleaning -> maid \[slave\] (15)
* etc

