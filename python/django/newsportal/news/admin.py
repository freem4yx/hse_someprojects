from django.contrib import admin

from .models import Article, Category, Comment, Tag


@admin.register(Article)
class ArticleAdmin(admin.ModelAdmin):
    list_display = ("title", "author", "category", "status", "created_at", "views")
    list_filter = ("status", "category", "tags", "created_at")
    search_fields = ("title", "content", "author__username")
    filter_horizontal = ("tags",)


    def save_model(self, request, obj, form, change):
        if not obj.slug:
            from django.utils.text import slugify

            obj.slug = slugify(obj.title)
            original_slug = obj.slug
            counter = 1
            while Article.objects.filter(slug=obj.slug).exclude(id=obj.id).exists():
                obj.slug = f"{original_slug}-{counter}"
                counter += 1
        super().save_model(request, obj, form, change)


@admin.register(Category)
class CategoryAdmin(admin.ModelAdmin):
    prepopulated_fields = {"slug": ("name",)}


@admin.register(Tag)
class TagAdmin(admin.ModelAdmin):
    prepopulated_fields = {"slug": ("name",)}


@admin.register(Comment)
class CommentAdmin(admin.ModelAdmin):
    list_display = ("author", "article", "created_at", "is_active")
    list_filter = ("is_active", "created_at")
    search_fields = ("content", "author__username", "article__title")
    actions = ["approve_comments", "disapprove_comments"]

    def approve_comments(self, request, queryset):
        queryset.update(is_active=True)
        self.message_user(request, "Комментарии одобрены")

    def disapprove_comments(self, request, queryset):
        queryset.update(is_active=False)
        self.message_user(request, "Комментарии скрыты")

    approve_comments.short_description = "Одобрить выбранные комментарии"
    disapprove_comments.short_description = "Скрыть выбранные комментарии"
