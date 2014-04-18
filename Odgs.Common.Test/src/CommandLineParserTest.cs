// <summary>
// CommandLineParser unit tests.
// </summary>

namespace Odgs.Common.Test
{
    using NUnit.Framework;

    /// <summary>
    /// CommandLineParser unit tests.
    /// </summary>
    [TestFixture]
    public class CommandLineParserTest
    {
        /// <summary>
        /// Various options and arguments.
        /// </summary>
        [Test]
        public void ComplexCommandLine()
        {
            CommandLine cl = CommandLine.Parse(new[]
                                                   {
                                                       "arg1",
                                                       "arg2",
                                                       "--option1",
                                                       "--option2",
                                                       "--option3=3",
                                                       "arg3", 
                                                       "--option4=value1",
                                                       "--option4=value2"
                                                   });

            Assert.That(cl.Arguments, Has.Count.EqualTo(3));
            Assert.That(cl.Options, Has.Count.EqualTo(4));

            Assert.That(cl.Arguments[0], Is.EqualTo("arg1"));

            Assert.That(cl.Options["option1"], Is.EqualTo(new string[] { }));
            Assert.That(cl.Options["option2"], Is.EqualTo(new string[] { }));
            Assert.That(cl.Options["option3"], Is.EqualTo(new[] { "3" }));

            // Values of multiple options with same name 
            // should have the same order as in command line.
            Assert.That(cl.Options["option4"], Is.EqualTo(new[] { "value1", "value2" }));
        }

        /// <summary>
        /// Option values with empty elements.
        /// </summary>
        [Test]
        public void ConsolidateOptionValuesEmptyItems()
        {
            var cl = CommandLine.Parse(new[]
                                           {
                                               "--option4=4_1,4_2;;4_3,,4_4,", 
                                           });

            cl.ConsolidateOptionValues("option4");

            Assert.That(cl.Options, Has.Count.EqualTo(1));

            // Consolidate removes empty entries.
            Assert.That(cl.Options["option4"], Is.EqualTo(new[] { "4_1", "4_2", "4_3", "4_4" }));
        }

        /// <summary>
        /// Tests ConsolidateOptionValues with mix of optins and values.
        /// </summary>
        [Test]
        public void ConsolidateOptionValuesMixed()
        {
            var cl = CommandLine.Parse(new[]
                                           {
                                               "--option0=,",
                                               "--option0=",
                                               "--option0",
                                               "--option3=3_1,3_2",
                                               "--option3=3_3",
                                               "--option4=4_1,4_2",
                                               "--option4=4_3;4_4"
                                           });

            cl.ConsolidateOptionValues("option0");
            cl.ConsolidateOptionValues("option3");
            cl.ConsolidateOptionValues("option4");

            Assert.That(cl.Options, Has.Count.EqualTo(3));

            // Consolidate removes empty entries.
            Assert.That(cl.Options["option0"], Is.EqualTo(new string[] { }));
            Assert.That(cl.Options["option3"], Is.EqualTo(new[] { "3_1", "3_2", "3_3" }));
            Assert.That(cl.Options["option4"], Is.EqualTo(new[] { "4_1", "4_2", "4_3", "4_4" }));
        }

        /// <summary>
        /// Tests ConsolidateOptionValues that same values in two instances of the same option do not get merged.
        /// </summary>
        [Test]
        public void ConsolidateOptionValuesMultipleItems()
        {
            var cl = CommandLine.Parse(new[]
                                           {
                                               "--option4=4_1,4_2,4_3,4_4", 
                                               "--option4=4_1,4_2,4_3,4_4", 
                                           });

            cl.ConsolidateOptionValues("option4");

            Assert.That(cl.Options, Has.Count.EqualTo(1));

            // Consolidate removes empty entries.
            Assert.That(
                        cl.Options["option4"],
                        Is.EqualTo(new[] { "4_1", "4_2", "4_3", "4_4", "4_1", "4_2", "4_3", "4_4" }));
        }

        /// <summary>
        /// Tests ConsolidateOptionValues with values specified in separate options.
        /// </summary>
        [Test]
        public void ConsolidateOptionValuesSeparate()
        {
            var cl = CommandLine.Parse(new[]
                                           {
                                               "--optionE", 
                                               "--optionE", 
                                               "--option0=", 
                                               "--option0=", 
                                               "--option1=1_1", 
                                               "--option2=2_1", 
                                               "--option2=2_2", 
                                               "--option3=3_1", 
                                               "--option3=3_2", 
                                               "--option3=3_3", 
                                           });

            cl.ConsolidateOptionValues("optionE");
            cl.ConsolidateOptionValues("option0");
            cl.ConsolidateOptionValues("option1");
            cl.ConsolidateOptionValues("option2");
            cl.ConsolidateOptionValues("option3");

            Assert.That(cl.Options, Has.Count.EqualTo(5));

            Assert.That(cl.Options["optionE"], Is.EqualTo(new string[] { }));

// Consolidate removes empty entries.
            Assert.That(cl.Options["option0"], Is.EqualTo(new string[] { }));
            Assert.That(cl.Options["option1"], Is.EqualTo(new[] { "1_1" }));
            Assert.That(cl.Options["option2"], Is.EqualTo(new[] { "2_1", "2_2" }));
            Assert.That(cl.Options["option3"], Is.EqualTo(new[] { "3_1", "3_2", "3_3" }));
        }

        /// <summary>
        /// Tests ConsolidateOptionValues with values separated by various default separators.
        /// </summary>
        [Test]
        public void ConsolidateOptionValuesWithSeparators()
        {
            var cl = CommandLine.Parse(new[]
                                           {
                                               "--option0=,", 
                                               "--option1=1_1", 
                                               "--option2=2_1;2_2", 
                                               "--option3=3_1,3_2,3_3", 
                                               "--option4=4_1,4_2;4_3,4_4", 
                                           });

            cl.ConsolidateOptionValues("option0");
            cl.ConsolidateOptionValues("option1");
            cl.ConsolidateOptionValues("option2");
            cl.ConsolidateOptionValues("option3");
            cl.ConsolidateOptionValues("option4");

            Assert.That(cl.Options, Has.Count.EqualTo(5));

            // Consolidate removes empty entries.
            Assert.That(cl.Options["option0"], Is.EqualTo(new string[] { }));
            Assert.That(cl.Options["option1"], Is.EqualTo(new[] { "1_1" }));
            Assert.That(cl.Options["option2"], Is.EqualTo(new[] { "2_1", "2_2" }));
            Assert.That(cl.Options["option3"], Is.EqualTo(new[] { "3_1", "3_2", "3_3" }));
            Assert.That(cl.Options["option4"], Is.EqualTo(new[] { "4_1", "4_2", "4_3", "4_4" }));
        }

        /// <summary>
        /// Tests custom separators for ConsolidateOptionValues.
        /// </summary>
        [Test]
        public void ConsolidateOptionValuesCustomSeparators()
        {
            var cl = CommandLine.Parse(new[]
                                           {
                                               "--option4=4_1#4_2*4_3*4_4", 
                                               "--option4=4_1,4_2,4_3,4_4", 
                                           });

            cl.ConsolidateOptionValues("option4", new[] { '#', '*' });

            Assert.That(cl.Options, Has.Count.EqualTo(1));

            Assert.That(
                        cl.Options["option4"],
                        Is.EqualTo(new[] { "4_1", "4_2", "4_3", "4_4", "4_1,4_2,4_3,4_4" }));
        }

        /// <summary>
        /// Tests for an empty command line.
        /// </summary>
        [Test]
        public void Empty()
        {
            CommandLine cl = CommandLine.Parse(new string[] { });
            Assert.That(cl.Arguments, Has.Count.EqualTo(0));
            Assert.That(cl.Options, Has.Count.EqualTo(0));
        }

        /// <summary>
        /// Option with explicit empty value.
        /// </summary>
        [Test]
        public void EmptyOptionValue()
        {
            CommandLine cl = CommandLine.Parse(new[] { "--option=" });
            Assert.That(cl.Arguments, Is.Empty);
            Assert.That(cl.Options, Has.Count.EqualTo(1));

            Assert.That(cl.Options["option"], Has.Count.EqualTo(1));
            Assert.That(cl.Options["option"], Is.EqualTo(new[] { string.Empty }));
        }

        /// <summary>
        /// Multiple instances of the same option with different values.
        /// </summary>
        [Test]
        public void MultipleSameOptionsWithValues()
        {
            CommandLine cl = CommandLine.Parse(new[] { "--option=value1", "--option=value2" });
            Assert.That(cl.Arguments, Is.Empty);
            Assert.That(cl.Options, Has.Count.EqualTo(1));

            Assert.That(cl.Options["option"], Has.Count.EqualTo(2));
            Assert.That(cl.Options["option"], Is.EqualTo(new[] { "value1", "value2" }));
        }

        /// <summary>
        /// Multiple instances of the same option with the same value.
        /// </summary>
        [Test]
        public void MultipleSameOptionsWithSameValue()
        {
            CommandLine cl = CommandLine.Parse(new[] { "--option=value1", "--option=value1" });
            Assert.That(cl.Arguments, Is.Empty);
            Assert.That(cl.Options, Has.Count.EqualTo(1));

            Assert.That(cl.Options["option"], Has.Count.EqualTo(2));
            Assert.That(cl.Options["option"], Is.EqualTo(new[] { "value1", "value1" }));
        }

        /// <summary>
        /// Tests for null command line.
        /// </summary>
        [Test]
        [ExpectedException(typeof( System.NullReferenceException ))]
        public void Null()
        {
            CommandLine.Parse(null);
        }

        /// <summary>
        /// Single argument processing.
        /// </summary>
        [Test]
        public void OneArgument()
        {
            CommandLine cl = CommandLine.Parse(new[] { "arg1" });
            Assert.That(cl.Arguments, Has.Count.EqualTo(1));
            Assert.That(cl.Options, Is.Empty);

            Assert.That(cl.Arguments, Is.EqualTo(new[] { "arg1" }));
        }

        /// <summary>
        /// Single options without value.
        /// </summary>
        [Test]
        public void OneOption()
        {
            CommandLine cl = CommandLine.Parse(new[] { "--option" });
            Assert.That(cl.Arguments, Is.Empty);
            Assert.That(cl.Options, Has.Count.EqualTo(1));

            Assert.That(cl.Options["option"], Has.Count.EqualTo(0));
        }

        /// <summary>
        /// Single option with one value.
        /// </summary>
        [Test]
        public void OneOptionOneValue()
        {
            CommandLine cl = CommandLine.Parse(new[] { "--option=value" });
            Assert.That(cl.Arguments, Is.Empty);
            Assert.That(cl.Options, Has.Count.EqualTo(1));

            Assert.That(cl.Options["option"], Has.Count.EqualTo(1));
            Assert.That(cl.Options["option"], Is.EqualTo(new[] { "value" }));
        }
    }
}

