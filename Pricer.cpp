#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>
#include <vector>
#include <string>
using namespace std;

/*+++++++++++ AUXILIARY FUNCTIONS +++++++++++ */
/*+++++++++++++++++++++++++++++++++++++++++++ */

double StringtoNum(string s)
{
	double val = 0.0;
	stringstream ss(s);
	ss >> val;
	return val;
}

string NumtoString(double i)
{
	stringstream ss;
	ss << i;
	return ss.str();
}

// Return unique order id of highest unvisited price bid
string findMaxOffer(map<string, vector<string>> &orderMap)
{
	double maxOffer = 0.0;
	string uniqueId = "";
	map<string, vector<string>>::iterator itr;
	for (itr = orderMap.begin(); itr != orderMap.end(); itr++)
	{
		double priceAmt = StringtoNum(itr->second[4]);
		// checks only unvisited prices to find max price
		if (priceAmt > maxOffer && itr->second.size() == 6)
		{	
			maxOffer = priceAmt;
			uniqueId = itr->second[2];
		}
	}
	// append dummy string value "1" of max price in orderMap to prevent findMaxOffer from 
	// marking same price as max over and over again
	auto it = orderMap.find(uniqueId);
	if (it != orderMap.end())
	{
		it->second.push_back("1");
	}
	return uniqueId;
}

// Return unique order id of lowest unvisited price bid
string findMinOffer(map<string, vector<string>> &orderMap)
{
	// initialize minOffer with first price from the map
	double minOffer = 0.0;
	string uniqueId = "";
	map<string, vector<string>>::iterator itr;
	for (itr = orderMap.begin(); itr != orderMap.end(); itr++)
	{
		double priceAmt = StringtoNum(itr->second[4]);
		// checks only unvisited prices to find min price
		if ((minOffer == 0.0 || priceAmt < minOffer) && itr->second.size() == 6)
		{	
			minOffer = priceAmt;
			uniqueId = itr->second[2];
		}
	}
	// append dummy string value "1" of min price in orderMap to prevent findMinOffer from 
	// marking same price as min over and over again
	auto it = orderMap.find(uniqueId);
	if (it != orderMap.end())
	{
		it->second.push_back("1");
	}
	return uniqueId;
}

// Each input string is saved into a vector
// Provides error checking by printing a warning to standard error if there is an error in input message
vector<string> parseMessage(string message)
{
	string buffer;
	// insert message string into a stream to parse
	stringstream ss(message);
	// create vector to hold parsed string tokens
	vector<string> tokens;

	while (ss >> buffer)
	{
		tokens.push_back(buffer);
	}
	return tokens;
}

string timestampParsed(string message)
{
	return parseMessage(message)[0];
}

string orderTypeParsed(string message)
{	
	// Returns either "A" for addOrder or "R" for reduceOrder
	return parseMessage(message)[1];
}

string orderIdParsed(string message)
{
	return parseMessage(message)[2];
}

string sideParsed(string message)
{
	string side = "";
	if (orderTypeParsed(message) == "A")
	{
		side = parseMessage(message)[3];
	}
	
	return side;
}

string priceParsed(string message)
{
	string price = "";
	if (orderTypeParsed(message) == "A")
	{
		price = parseMessage(message)[4];
	}

	return price;
}

string sizeParsed(string message)
{
	string size = "";
	if (orderTypeParsed(message) == "A")
	{
		size = parseMessage(message)[5];
	}
	else
	{
		size = parseMessage(message)[3];
	}

	return size;
}

/*++++++++++++++ CLASS PRICER +++++++++++++++ */
/*+++++++++++++++++++++++++++++++++++++++++++ */

class Pricer
{
public:
	Pricer(int targetSize);
	// Accessors
	int getTargetSize() const;
	int getBidCount() const;
	int getAskCount() const;
	string getTimestamp() const;
	string getAction() const;
	string getTotalExpense() const;
	string getTotalIncome() const;

	//Mutators 
	void setTimeStamp(string timestamp);
	void setAction(string action);
	void setTotalExpense(string total);
	void setTotalIncome(string total);
	void addOrder(string message);
	void reduceOrder(string message);
	void printOutput(string action);
	void Flush(map<string, vector<string>> &orderMap);
private:
	int target_size;
	int bidCount;
	int askCount;
	string m_timestamp;
	string m_action;
	string m_expense;
	string m_income;
	map<string, vector<string>> bidMap;
	map<string, vector<string>> askMap;
};

// Initialize constructor with a given target-size
// book initially contains no orders and buying expense and selling income are considered to start at "NA"
Pricer::Pricer(int targetSize) : m_timestamp(""), m_action(""), m_expense("NA"), m_income("NA"), 
	bidCount(0), askCount(0) 
{
	target_size = targetSize;
}

int Pricer::getTargetSize() const
{
	return target_size;
}

int Pricer::getBidCount() const
{
	return bidCount;
}

int Pricer::getAskCount() const
{
	return askCount;
}

string Pricer::getTimestamp() const
{
	return m_timestamp;
}

string Pricer::getAction() const
{
	return m_action;
}

string Pricer::getTotalExpense() const
{
	return m_expense;
}

string Pricer::getTotalIncome() const
{
	return m_income;
}

void Pricer::setTimeStamp(string timestamp)
{
	m_timestamp = timestamp;
}

void Pricer::setAction(string action)
{
	m_action = action;
}

void Pricer::setTotalExpense(string total)
{
	m_expense = total;
}

void Pricer::setTotalIncome(string total)
{
	m_income = total;
}

// Function that adds order to book
void Pricer::addOrder(string message)
{
	vector<string> tokens = parseMessage(message);
	string timestamp = timestampParsed(message);
	string id = orderIdParsed(message);
	string side = sideParsed(message);
	string price = priceParsed(message);
	string size = sizeParsed(message);

	// buy order (bid)
	// total income
	if (side == "B")
	{
		bidMap.insert(pair <string, vector<string>> (id, tokens));
		bidCount += StringtoNum(size);

		// You can sell target-size shares, preferably at most expensive price
		if (getBidCount() >= getTargetSize())
		{
			int bidsLeft = getTargetSize();
			double totalCost = 0.0;
			// Keep selling shares until number of bids
			while (bidsLeft > 0)
			{	
				string key = findMaxOffer(bidMap);
				auto it = bidMap.find(key);
				if (it != bidMap.end())
				{
					// if bid size of an order exceeds number of bids left to add
					if (StringtoNum(it->second[5]) > bidsLeft)
					{
						totalCost += StringtoNum(it->second[4]) * bidsLeft;
						bidsLeft = 0;
					}
					// if bid size of an order is smaller than number of bids left to add
					else
					{
						totalCost += StringtoNum(it->second[4]) * StringtoNum(it->second[5]);
						bidsLeft -= StringtoNum(it->second[5]);
					}
				}
				else
				{
					bidsLeft = 0;
				}
			}

			if (StringtoNum(getTotalIncome()) != totalCost)
			{
				setTimeStamp(timestamp);
				setAction("S");
				setTotalIncome(NumtoString(totalCost));
				printOutput(getAction());
			}
		}

		Flush(bidMap);
	}
	// sell order (ask)
	// total expense
	else
	{
		askMap.insert(pair <string, vector<string>> (id, tokens));
		askCount += StringtoNum(size);

		// You can buy target-size shares, preferably at cheapest price  
		if (getAskCount() >= getTargetSize())
		{
			int asksLeft = getTargetSize();
			double totalCost = 0.0;
			// Keep selling shares until number of bids
			while (asksLeft > 0)
			{	
				string key = findMinOffer(askMap);
				auto it = askMap.find(key);
				if (it != askMap.end())
				{
					// if bid size of an order exceeds number of bids left to add
					if (StringtoNum(it->second[5]) > asksLeft)
					{
						totalCost += StringtoNum(it->second[4]) * asksLeft;
						asksLeft = 0;
					}
					// if bid size of an order is smaller than number of bids left to add
					else
					{
						totalCost += StringtoNum(it->second[4]) * StringtoNum(it->second[5]);
						asksLeft -= StringtoNum(it->second[5]);
					}
				}
				else
				{
					asksLeft = 0;
				}
			}

			if (StringtoNum(getTotalExpense()) != totalCost)
			{
				setTimeStamp(timestamp);
				setAction("B");
				setTotalExpense(NumtoString(totalCost));
				printOutput(getAction());
			}
		}

		Flush(askMap);
	}
}

// Function that reduces order 
void Pricer::reduceOrder(string message)
{
	string timestamp = timestampParsed(message);
	string id = orderIdParsed(message);
	string size = sizeParsed(message);

	auto bidSearch = bidMap.find(id);
	auto askSearch = askMap.find(id);

	// if key exists in bidMap
	if (bidSearch != bidMap.end())
	{
		bidCount -= StringtoNum(size);
		// if size is >= existing size of the book, the order is removed from the book
		if (StringtoNum(size) >= getBidCount())
		{
			bidMap.erase(id);
		}
		// reduces size of order by size amount
		else
		{	
			int newSize = StringtoNum(bidSearch->second[5]) - StringtoNum(size);
			bidSearch->second[5] = NumtoString(newSize);
		}

		if (getBidCount() >= getTargetSize())
		{
			int bidsLeft = getTargetSize();
			double totalCost = 0.0;
			// Keep selling shares until number of bids
			while (bidsLeft > 0)
			{	
				string key = findMaxOffer(bidMap);
				auto it = bidMap.find(key);
				if (it != bidMap.end())
				{
					// if bid size of an order exceeds number of bids left to add
					if (StringtoNum(it->second[5]) > bidsLeft)
					{
						totalCost += StringtoNum(it->second[4]) * bidsLeft;
						bidsLeft = 0;
					}
					// if bid size of an order is smaller than number of bids left to add
					else
					{
						totalCost += StringtoNum(it->second[4]) * StringtoNum(it->second[5]);
						bidsLeft -= StringtoNum(it->second[5]);
					}
				}
				else
				{
					bidsLeft = 0;
				}
			}

			if (StringtoNum(getTotalIncome()) != totalCost)
			{
				setTimeStamp(timestamp);
				setAction("S");
				setTotalIncome(NumtoString(totalCost));
				printOutput(getAction());
			}
		}
		else
		{
			if (getTotalIncome() != "NA")
			{
				setTimeStamp(timestamp);
				setAction("S");
				setTotalIncome("NA");
				printOutput(getAction());			
			}	
		}

		Flush(bidMap);
	}

	// if key exists in askMap
	if (askSearch != askMap.end())
	{
		askCount -= StringtoNum(size);
		// if size is >= existing size of the book, the order is removed from the book
		if (StringtoNum(size) >= getAskCount())
		{
			askMap.erase(id);
		}
		// reduces size of order by size amount
		else
		{	
			int newSize = StringtoNum(askSearch->second[5]) - StringtoNum(size);
			askSearch->second[5] = NumtoString(newSize);
		}

		if (getAskCount() >= getTargetSize())
		{
			int asksLeft = getTargetSize();
			double totalCost = 0.0;
			// Keep selling shares until number of bids
			while (asksLeft > 0)
			{	
				string key = findMinOffer(askMap);
				auto it = askMap.find(key);
				if (it != askMap.end())
				{
					// if bid size of an order exceeds number of bids left to add
					if (StringtoNum(it->second[5]) > asksLeft)
					{
						totalCost += StringtoNum(it->second[4]) * asksLeft;
						asksLeft = 0;
					}
					// if bid size of an order is smaller than number of bids left to add
					else
					{
						totalCost += StringtoNum(it->second[4]) * StringtoNum(it->second[5]);
						asksLeft -= StringtoNum(it->second[5]);
					}
				}
				else
				{
					asksLeft = 0;
				}
			}

			if (StringtoNum(getTotalExpense()) != totalCost)
			{
				setTimeStamp(timestamp);
				setAction("B");
				setTotalExpense(NumtoString(totalCost));
				printOutput(getAction());
			}
		}
		else
		{
			if (getTotalExpense() != "NA")
			{
				setTimeStamp(timestamp);
				setAction("B");
				setTotalExpense("NA");
				printOutput(getAction());
			}
		}

		Flush(askMap);
	}
}

// prints in formatted output
// price is always set to a precision of 2 decimal places unless result must be "NA"
void Pricer::printOutput(string action)
{
	// print output using totalExpense
	if (action == "B")
	{
		if (getTotalExpense() != "NA")
		{
			double total = StringtoNum(getTotalExpense());
			cout << getTimestamp() + " " + getAction() + " ";
			cout << fixed << setprecision(2) << total << endl;
		}
		else
		{
			cout << getTimestamp() + " " + getAction() + " " + getTotalExpense() << endl;
		}
	}
	// print output using totalIncome
	else
	{
		if (getTotalIncome() != "NA")
		{
			double total = StringtoNum(getTotalIncome());
			cout << getTimestamp() + " " + getAction() + " ";
			cout << fixed << setprecision(2) << total << endl;
		}
		else
		{
			cout << getTimestamp() + " " + getAction() + " " + getTotalIncome() << endl;
		}
	}
}

// Clears out any "1" values that were appended to the vector<string> of a key from addOrder
// Ensures that each call to addOrder produces a fresh map for findMaxOffer or findMinOffer
void Pricer::Flush(map<string, vector<string>> &orderMap)
{
	map<string, vector<string>>::iterator itr;
	for (itr = orderMap.begin(); itr != orderMap.end(); itr++)
	{
		// a "1" was appended, so remove the last element
		if (itr->second.size() == 7 && itr->second[6] == "1")
		{
			(itr->second).erase(itr->second.begin()+6);
		}
	}
}

int main(int argc, char *argv[])
{
	//ifstream instream;
	// pass target-size as argument
	stringstream ss(argv[1]);
	int targetSize;
	// extract formatted input, check if conversion is successful 
	ss >> targetSize;
	//cout << targetSize << endl;

	Pricer p(targetSize);
	string linebuffer;
	while (getline(cin, linebuffer))
	{
		//cout << linebuffer.size() << endl;
		if (orderTypeParsed(linebuffer) == "A")
		{
			p.addOrder(linebuffer);
		}
		else
		{
			p.reduceOrder(linebuffer);
		}
	}

	return 0;
}