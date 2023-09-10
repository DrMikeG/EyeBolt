The last update on this project was about a month ago, at which point I had swapped the motor on my direct drive turn table.

I was using a 28BYJ stepper, which had terrible backlash, and a lack of usable rotation positions.
I switched it up to this smaller FIT0503 Micro Metal Geared Stepper Motor.

This has a precision metal gear box with a 1:100 reduction.

Because it was smaller than the motor it was replacing, I could just print some shims to fit it into the exiting mechanism.

When I installed this, there was noticeably less backlash for the table, and many more steps per revolution.

But I noticed immeidatley, that using the published number of steps per revolution, the turn table wasn't turning a full revolution.

So, it was time to determine the number of steps empirically.

So the experiemnt setup is to run capture and image, and then perform a full revolution of the table, an to repeat this a number of times. When viewed as a timelapse video, the sequence of images should show stationary image, but and under or over rotation will be represented as procession or recessions in the frames.

The data sheet step angle is 18 degrees, so 20 steps per revolution of the motor shaft. With a 1:100 gear box, that becomes 2000 steps per revolution. This sequence of images (which is looping) show that for 1984 steps is one that one full revolution of the table.

For this sequence we reduced the steps between captures from 1984 to 1976, and now the table is under rotating.

Not being able to position the table accuratley was where we started, and whilst it has improved witht he new motor, the number of steps per revolution 1981 is almost a prime number, and therefore difficult to divide into even steps.

I took a bit of a break, and decided that what I wanted to do was redesign the turn table to be more fit for purpose.   

In the new design I'm now using the same motor, but rather than directly driving the table, I am using a special work gear.

The table has 100 teeth around the outside, which correspond to 100 diffent orientation positions.
 
 The new table is very slow - but it gives exact ly 100 fixed rotation positions.
 Moving from one rotation position to the next requires 1981 steps of the motor, and the position is locked for about 500 of the steps, so the stepper and gearbox can easily afford to be off by +/- 20 steps, as long the error is cyclic not cumulative.

Now, with 200 steps per tooth and 100 steps per revolution, it takes the table almost 3 hours to do 10 revolutions. This sequence of image shows the stability of the period of the new table design.

And yes, I did just chuck whatever junk I had to hand onto the table at the start of this experiment.