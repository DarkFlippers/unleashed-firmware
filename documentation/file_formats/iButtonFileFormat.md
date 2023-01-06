# iButton key file format

## Example

```
Filetype: Flipper iButton key
Version: 1
# Key type can be Cyfral, Dallas or Metakom
Key type: Dallas
# Data size for Cyfral is 2, for Metakom is 4, for Dallas is 8
Data: 12 34 56 78 9A BC DE F0
```

## Description

Filename extension: `.ibtn`

The file stores a single iButton key of the type defined by the `Key type` parameter.

### Version history

1. Initial version.

### Format fields

| Name     | Description                                  |
| -------- | -------------------------------------------- |
| Key type | Currently supported: Cyfral, Dallas, Metakom |
| Data     | Key data (HEX values)                        |
