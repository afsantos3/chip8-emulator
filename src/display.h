class Display
{
public:
  Display();
  void Update(const uint8_t* buffer);

private:
  void Print(const uint8_t* buffer);
  void Clear();
};