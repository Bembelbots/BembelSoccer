def bootstrap():
    """
    Adds python_libs to path containing bembelapi and logger config.
    """
    import sys
    from pathlib import Path

    softwareDir = Path(__file__).parents[1]
    representationsDir = softwareDir.parent.joinpath("soccer/build/v6/include/representations")
    sys.path.append(str(softwareDir.joinpath("python_libs")))
    sys.path.append(str(softwareDir.joinpath("python_libs/third_party")))
    sys.path.append(str(representationsDir))
