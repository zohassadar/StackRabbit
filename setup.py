from setuptools import setup, Extension



def main():
    setup(name="stackrabbit",
          version="1.0.1",
          description="Python interface for StackRabbit",
          ext_modules=[Extension("stackrabbit", ["src/cpp_modules/src/pymodule.cpp"])])

if __name__ == "__main__":
    main()
