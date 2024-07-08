using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Data;
using System.Text.RegularExpressions;

namespace BLContentBuilder
{
    public static class SplitHelper
    {
        enum FORMATDATAROW { XLSXTOLISTMAIN, }

        private static readonly char[] SPLITS = new char[] { ';', ',' };
        private static readonly char[] BRACKETS = { '{', '(', ')', '}' };

        private static readonly Dictionary<FORMATDATAROW, Tuple<string, object[]>> FORMATS_DATAROW = new Dictionary<FORMATDATAROW, Tuple<string, object[]>>()
        {
            { FORMATDATAROW.XLSXTOLISTMAIN, new Tuple<string, object[]>( "{0}_{1}_{2}", new object[] { "id", "code_name", "local_name" } ) },
        };

        private static string GetFormatDataRow(FORMATDATAROW typeDataRow, DataRow r)
        {
            string formatString = FORMATS_DATAROW[typeDataRow].Item1;
            object[] fields = FORMATS_DATAROW[typeDataRow].Item2;
            object[] arguments = new object[fields.Length];

            for (int i = 0; i < fields.Length; ++i)
            {
                arguments[i] = r[(string)fields[i]];
            }

            return string.Format(formatString, arguments);
        }

        public static int GetCountArgs(string text)
        {
            if (true == text.IsNullOrEmpty())
                return 0;

            int count = 0;
            for(count = 0; count <= 1 << 16; ++count)
            {
                string argToFind = "(" + count + ")";
                if (false == text.Contains(argToFind))
                    break;
            }

            return count;
        }

        public static string FormatItemListMain(DataRow r)
        {
            return GetFormatDataRow(FORMATDATAROW.XLSXTOLISTMAIN, r);
        }

        public static bool IsValidateMerged(string text)
        {
            if (true == text.IsNullOrEmpty())
                return true;
            
            if (BRACKETS.Any(b=> { return text.Contains(b); }))
            {
                return IsBracketBalanced(text);
            }

            return true;
        }

        public static bool IsBracketBalanced(string text)
        {
            if (true == text.IsNullOrEmpty())
            {
                return false;
            }
            var stack = new Stack<char>();

            foreach (char chr in text)
                if (BRACKETS.Any(x => x == chr))
                    stack.Push(chr);

            var reverseStack = stack.Reverse();
            return reverseStack.SequenceEqual(stack, BalancedParanthesisComparer.Instance);
        }

        private sealed class BalancedParanthesisComparer : EqualityComparer<char>
        {
            private static readonly BalancedParanthesisComparer _instance = new BalancedParanthesisComparer();

            private BalancedParanthesisComparer() { }

            public static BalancedParanthesisComparer Instance { get { return _instance; } }

            public override int GetHashCode(char obj)
            {
                return int.MinValue;
            }

            public override bool Equals(char x, char y)
            {
                if ((x == '(' && y == ')') || (y == '(' && x == ')'))
                    return true;
                if (x == '{' && y == '}' || (y == '{' && x == '}'))
                    return true;

                return false;
            }
        }

        public static string[] GetSplitBracketSimpleDefault(string text)
        {
            string[] splitted = GetSplitBracketSimple(text);
            if (true == splitted.IsNullOrEmpty())
            {
                return null;
            }

            if (splitted.Length >= 2)
            {
                GLog.LogError("(splitted.Length >= 2)/GetSplitBracketSingle/text:" + text);
                return null;
            }

            return GetSplitSingle(splitted[0]);
        }

        public static string[] GetSplitBracketSimple(string text)
        {
            if (false == IsBracketBalanced(text))
            {
                return null;
            }

            Regex regex = new Regex(@" \(         
                              (?>                
                                  [^()]+         
                                |                
                                  \( (?<DEPTH>)  
                                |                
                                  \) (?<-DEPTH>) 
                              )*                 
                              (?(DEPTH)(?!))     
                            \)                   ",
            RegexOptions.IgnorePatternWhitespace);

            List<string> splitted = new List<string>();
            foreach (Match c in regex.Matches(text))
            {
                splitted.Add((c.Value.Trim('(', ')')));
            }

            if (false == splitted.IsNullOrEmpty())
            {
                return splitted.Select(s => { return null != s ? s.Trim() : s; }).ToArray();
            }

            return null;
        }

        public static string[] GetSplitBracketSingle(string text, bool isOnlyBracketed = true, bool isReturnWithBracket = true)
        {
            if (false == IsBracketBalanced(text))
            {
                return null;
            }

            Regex regex = null;

            if (true == isOnlyBracketed)
            {
                regex = new Regex(@"\[[^]]*]|\{[^}]*}", RegexOptions.IgnorePatternWhitespace);
            }
            else
            {
                regex = new Regex(@"\[[^]]*]|\{[^}]*}|[^,]+", RegexOptions.IgnorePatternWhitespace);
            }

            string[] result = null;
            if (true == isReturnWithBracket)
            {
                result = (from Match m in regex.Matches(text)
                 select m.Value)
                .ToArray();
            }
            else
            {
                result = (from Match m in regex.Matches(text)
                 select m.Value.Trim('(', ')').Trim('{', '}'))
                .ToArray();
            }

            return result;
        }

        public static List<string[]> GetSplit(string text)
        {
            string test_ = text;
            if (true == test_.IsNullOrEmpty())
            {
                return null;
            }

            test_ = test_.Replace("{", "");
            test_ = test_.Replace("}", "");

            if (false == test_.Contains("(") || false == test_.Contains(")"))
            {
                string[] singleSplits = GetSplitSingle(test_);
                if (true == singleSplits.IsNullOrEmpty())
                    return null;
                else
                    return new List<string[]>() { singleSplits, };
            }

            List<string> singleMerges = new List<string>();
            bool isOpen = false; string appends = string.Empty;
            for(int i = 0; i < test_.Length; ++i)
            {
                char c = test_[i];
                if ('(' == c)
                {
                    appends = string.Empty;
                    isOpen = true; continue;
                }
                else if (')' == c)
                {
                    if (false == appends.IsNullOrEmpty())
                    {
                        singleMerges.Add(appends.Trim());
                    }
                    isOpen = false; continue;
                }

                if (true == isOpen)
                    appends += c;
            }

            if (true == singleMerges.IsNullOrEmpty())
                return null;

            List<string[]> splitsList = new List<string[]>();
            singleMerges.Each(
                s =>
                {
                    string[] splits = GetSplitSingle(s);
                    if (false == splits.IsNullOrEmpty())
                        splitsList.Add(splits);
                });

            return splitsList.IsNullOrEmpty() ? null : splitsList;
        }

        public static string GetTrimBrackets(string text)
        {
            bool isBracketed = text.Contains("{") || text.Contains("}") || text.Contains("(") || text.Contains(")");
            if (true == isBracketed)
            {
                text = text.Replace("{", "");
                text = text.Replace("}", "");
                text = text.Replace("(", "");
                text = text.Replace(")", "");
            }
            return text;
        }

        public static string[] GetSplitSingle(string text)
        {
            string text_ = text;
            if (true == text_.IsNullOrEmpty())
            {
                return null;
            }

            text_ = GetTrimBrackets(text_);

            if (false == text_.Any(c=> { return SPLITS.Contains(c); }))
                return new string[] { text_.Trim() };

            string textToUse = text_.Trim();

            if (true == textToUse.IsNullOrEmpty())
            {
                return null;
            }

            char split = SPLITS.FirstOrDefault(s => textToUse.Contains(s) );

            string[] trimmeds = textToUse.Split(split).Select(s=> s.Trim()).ToArray();

            if (true == trimmeds.IsNullOrEmpty())
            {
                return null;
            }

            return trimmeds;
        }

        public static string[] GetSplitExceptChoice(string[] splits)
        {
            if (true == splits.IsNullOrEmpty())
                return null;

            if (splits.Length == 1)
                return null;

            string[] splitsNew = new string[splits.Length - 1];
            Array.Copy(splits, 1, splitsNew, 0, splits.Length - 1);
            return splitsNew;
        }

        public static string GetMergeBracketMulti(List<string> textBundles, string defaultValueIfNull = null)
        {
            if (true == textBundles.IsNullOrEmpty())
            {
                return defaultValueIfNull;
            }

            return GetMergeBracketMulti(new List<List<string>> { textBundles }, defaultValueIfNull);
        }

        public static string GetMergeBracketMulti(List<List<string>> textBundles, string defaultValueIfNull = null)
        {
            if (true == textBundles.IsNullOrEmpty())
            {
                return defaultValueIfNull;
            }

            List<List<string>> textBundlesToUse = new List<List<string>>();
            foreach(List<string> texts in textBundles)
            {
                List<string> textsToUse = new List<string>();
                foreach (string t in texts)
                {
                    if (t.IsNullOrEmpty())
                        break;

                    textsToUse.Add(t);
                }

                if (false == textsToUse.IsNullOrEmpty())
                {
                    textBundlesToUse.Add(textsToUse);
                }
            }

            if (textBundlesToUse.IsNullOrEmpty())
                return defaultValueIfNull;

            string output = "{";
            bool isMergedAtLeast1More = false;
            foreach(List<string> texts in textBundlesToUse)
            {
                if (true == texts.IsNullOrEmpty())
                {
                    continue;
                }

                output += "(";
                for(int i = 0; i < texts.Count; ++i)
                {
                    string t = texts[i];

                    if (true == t.IsNullOrEmpty())
                        break;

                    isMergedAtLeast1More = true;
                    output += t;
                    if (i != texts.Count - 1)
                    {
                        output += ", ";
                    }
                }
                output += ")";
                if (texts != textBundlesToUse.Last())
                {
                    output += ",";
                }
            }
            output += "}";

            return (true == isMergedAtLeast1More)? output : null;
        }

        public static string GetMergeBracketSingle(List<string> texts, bool isTrimEnd = true)
        {
            if (true == texts.IsNullOrEmpty())
            {
                return null;
            }

            List<string> textsToUse = new List<string>();
            foreach(string t in texts)
            {
                if (t.IsNullOrEmpty())
                    break;

                textsToUse.Add(t);
            }

            if (true == textsToUse.IsNullOrEmpty())
                return null;

            string output = "{(";
            bool isMergedAtLeast1More = false;
            foreach (string t in textsToUse)
            {
                isMergedAtLeast1More = true;
                output += t;
                if (t != textsToUse.Last())
                {
                    output += ", ";
                }
            }
            output += ")}";

            return (true == isMergedAtLeast1More) ? output : null;
        }

        public static string GetMergeSingle(List<string> texts, bool isExceptNullItem = true, string defaultValue = null, char seperator = ',')
        {
            if (true == texts.IsNullOrEmpty())
                return defaultValue;

            List<string> textsToUse = new List<string>();
            foreach (string t in texts)
            {
                if (t.IsNullOrEmpty())
                    break;

                textsToUse.Add(t);
            }

            if (true == textsToUse.IsNullOrEmpty())
                return defaultValue;

            string output = string.Empty;

            foreach (string t in textsToUse)
            {
                output += t;
                if (t != textsToUse.Last())
                {
                    output += seperator + " ";
                }
            }

            return output;
        }

    }
}
