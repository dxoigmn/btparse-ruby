class BibTeX
  attr_reader :entries
end

class BibTeX::Entry
  attr_reader :type, :key, :fields

  def to_s
    str = "@#{type}{#{key},\n"

    fields.each do |name, value|
      str += "\t#{BibTeX.purify(name)} = "
      if name == "author"
        str += BibTeX.parse_author(value).inspect
      else
        str += "'" + BibTeX.purify(value) + "'"
      end
      str += "\n"
    end

    str += "}"
  end
end
